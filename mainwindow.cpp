#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "folderdialog.h"
#include "iconfactory.h"
#include "iconprovider.h"
#include "ribbongroup.h"

#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QTextEdit>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QDir>
#include <QAction>
#include <QStatusBar>
#include <QProcess>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>
#include <QStyleFactory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Header bar ---
    headerLabel_ = new QLabel("Home", this);
    headerLabel_->setAlignment(Qt::AlignCenter);
    headerLabel_->setStyleSheet("QLabel { background:#202020; color:#66ccff; padding:10px; font-size:16px; font-weight:bold; }");

    themeToggle = new QCheckBox("Dark Mode", this);
    themeToggle->setChecked(false);
    themeToggle->setStyleSheet(R"(
        QCheckBox::indicator {
            width: 12px;
            height: 12px;
            border: 1px solid black;
        }
    )");
    connect(themeToggle, &QCheckBox::toggled, this, &MainWindow::togglePalette);

    auto* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(headerLabel_);
    headerLayout->addWidget(themeToggle);

    auto* headerWidget = new QWidget(this);
    headerWidget->setLayout(headerLayout);

    // --- File system model ---
    fsModel_ = new QFileSystemModel(this);
    fsModel_->setRootPath(QDir::homePath());
    fsModel_->setReadOnly(false);
    auto* provider = new IconProvider();
    fsModel_->setIconProvider(provider);

    tree_ = new QTreeView(this);
    tree_->setModel(fsModel_);
    tree_->setHeaderHidden(false);
    tree_->setAnimated(true);
    tree_->setExpandsOnDoubleClick(true);
    tree_->setUniformRowHeights(true);

    // Connect expansion/collapse to provider state
    connect(tree_, &QTreeView::expanded, this, [this, provider](const QModelIndex &index) {
        if (fsModel_->isDir(index))
            provider->setExpanded(fsModel_->filePath(index), true);
    });
    connect(tree_, &QTreeView::collapsed, this, [this, provider](const QModelIndex &index) {
        if (fsModel_->isDir(index))
            provider->setExpanded(fsModel_->filePath(index), false);
    });



    // --- List view ---
    list_ = new QListView(this);
    list_->setModel(fsModel_);
    list_->setViewMode(QListView::ListMode);
    list_->setUniformItemSizes(true);

    // --- Preview ---
    preview_ = new QTextEdit(this);
    preview_->setReadOnly(true);

    // --- Vertical split: list + preview ---
    auto* verticalSplit = new QSplitter(Qt::Vertical, this);
    verticalSplit->addWidget(list_);
    verticalSplit->addWidget(preview_);

    // --- Main split: tree + verticalSplit ---
    auto* mainSplit = new QSplitter(this);
    mainSplit->addWidget(tree_);
    mainSplit->addWidget(verticalSplit);

    // --- Central layout ---
    auto* centralLayout = new QVBoxLayout;
    centralLayout->setContentsMargins(0,0,0,0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(headerWidget);
    centralLayout->addWidget(mainSplit);

    auto* central = new QWidget(this);
    central->setLayout(centralLayout);
    setCentralWidget(central);

    // --- Status bar ---
    statusBar()->showMessage("Ready");
    statusBar()->setStyleSheet("QStatusBar { background:#202020; color:#66ccff; }");

    setupActions();
    setupConnections();

    tree_->setRootIndex(fsModel_->index(QDir::homePath()));
    list_->setRootIndex(fsModel_->index(QDir::homePath()));

    resize(1200, 800); // modern widescreen default
    setMinimumSize(1000, 700);
}

void MainWindow::setupActions() {
    tb = addToolBar("Ribbon");
    tb->setMovable(false);

    QSize iconSize(32,32);
    QColor brandColor = QColor(Qt::black);

    QVector<QPair<QString, QString>> fileActions = {
        {"Open",   ":/icons/icons/open_in_new.svg"},
        {"Rename", ":/icons/icons/rename.svg"},
        {"Delete", ":/icons/icons/delete.svg"}
    };

    QVector<QPair<QString, QString>> navActions = {
        {"Browse", ":/icons/icons/folder-search.svg"}
    };

    fileGroup = new RibbonGroup("File Actions", fileActions, this);
    fileGroup->updateIcons(brandColor, iconSize);

    navGroup = new RibbonGroup("Navigation", navActions, this);
    navGroup->updateIcons(brandColor, iconSize);

    tb->addWidget(fileGroup);
    tb->addSeparator();
    tb->addWidget(navGroup);


    // Connect signals
    connect(fileGroup, &RibbonGroup::actionTriggered, this, [this](const QString& name){
        if (name == "Open")   openSelected();
        if (name == "Rename") renameSelected();
        if (name == "Delete") deleteSelected();
    });
    connect(navGroup, &RibbonGroup::actionTriggered, this, [this](const QString& name){
        if (name == "Browse") {
            auto* provider = static_cast<IconProvider*>(fsModel_->iconProvider());
            bool darkMode = provider && provider->darkMode();

            // Get highlighted index from tree or list
            QModelIndex idx = tree_->currentIndex();
            if (!idx.isValid()) idx = list_->currentIndex();

            QString currentPath;
            if (idx.isValid()) {
                QFileInfo info(fsModel_->filePath(idx));
                if (info.isDir()) {
                    currentPath = info.absoluteFilePath();   // folder directly
                } else {
                    currentPath = info.absolutePath();       // parent folder if file
                }
            } else {
                // fallback to root if nothing highlighted
                currentPath = fsModel_->filePath(tree_->rootIndex());
            }

            // 🔹 Open dialog starting at that folder
            FolderDialog dlg(currentPath, this);
            dlg.setDarkMode(provider);

            if (dlg.exec() == QDialog::Accepted) {
                QString dir = dlg.selectedPath();
                if (!dir.isEmpty()) {
                    // navigate explorer into chosen folder
                    fsModel_->setRootPath(dir);
                    setRootPath(dir);
                }
            }
        }
    });

}


void MainWindow::openSelected() {
    const auto idx = list_->currentIndex().isValid() ? list_->currentIndex() : tree_->currentIndex();
    if(!idx.isValid()) return;
    const auto path = fsModel_->filePath(idx);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::revealInExplorer() {
    const auto idx = list_->currentIndex().isValid() ? list_->currentIndex() : tree_->currentIndex();
    if(!idx.isValid()) return;
    const auto path = fsModel_->filePath(idx);
#ifdef Q_OS_WIN
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(path)});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
#endif
}

void MainWindow::renameSelected() {
    const auto idx = list_->currentIndex().isValid() ? list_->currentIndex() : tree_->currentIndex();
    if(!idx.isValid()) return;
    const auto oldInfo = QFileInfo(fsModel_->filePath(idx));
    const auto newName = QInputDialog::getText(this, "Rename", "New name:", QLineEdit::Normal, oldInfo.fileName());
    if(newName.isEmpty()) return;
    const auto newPath = oldInfo.dir().absoluteFilePath(newName);
    if (!QFile::rename(oldInfo.absoluteFilePath(), newPath)) {
        QMessageBox::warning(this, "Rename failed", "Could not rename file.");
    } else {
        auto dirIndex = fsModel_->index(oldInfo.dir().absolutePath());
        list_->setRootIndex(dirIndex);
        tree_->setRootIndex(dirIndex);
    }
}

void MainWindow::deleteSelected() {
    const auto idx = list_->currentIndex().isValid() ? list_->currentIndex() : tree_->currentIndex();
    if(!idx.isValid()) return;
    const auto path = fsModel_->filePath(idx);
    const auto confirm = QMessageBox::question(this, "Delete", QString("Delete:\n%1 ?").arg(path));
    if(confirm != QMessageBox::Yes) return;
    QFileInfo info(path);
    bool ok = false;
    if(info.isDir()){
        QDir dir(path);
        ok = dir.removeRecursively();
    } else {
        ok = QFile::remove(path);
    }
    if(!ok) QMessageBox::warning(this, "Delete failed", "Could not delete.");
}

void MainWindow::setRootPath(const QString& path) {
    const auto rootIdx = fsModel_->index(path);
    tree_->setRootIndex(rootIdx);
    list_->setRootIndex(rootIdx);
    statusBar()->showMessage(path);
}

void MainWindow::setupConnections() {
    selModel_ = tree_->selectionModel();
    connect(selModel_, &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                list_->setRootIndex(current);
                statusBar()->showMessage(fsModel_->filePath(current));
            });

    connect(list_, &QListView::doubleClicked, this, [this](const QModelIndex& idx){
        if (!idx.isValid()) return;
        const auto path = fsModel_->filePath(idx);
        if (fsModel_->isDir(idx)) {
            list_->setRootIndex(idx);
        } else {
            openSelected();
        }
    });

    // Preview text files when selection changes
    connect(list_->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&){
                const auto path = fsModel_->filePath(current);
                QFile f(path);
                if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    auto data = f.read(64 * 1024); // limit preview size
                    preview_->setPlainText(QString::fromUtf8(data));
                } else {
                    preview_->clear();
                }
            });

    connect(selModel_, &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                QString path = fsModel_->filePath(current);
                headerLabel_->setText(path);

                auto* effect = new QGraphicsColorizeEffect(headerLabel_);
                headerLabel_->setGraphicsEffect(effect);
                auto* anim = new QPropertyAnimation(effect, "color");
                anim->setDuration(600);
                anim->setStartValue(QColor(0,180,255));
                anim->setEndValue(QColor(0,0,0,0));
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            });
    auto* provider = static_cast<IconProvider*>(fsModel_->iconProvider());

    connect(tree_, &QTreeView::expanded, this, [this, provider](const QModelIndex &index) {
        if (fsModel_->isDir(index)) {
            provider->setExpanded(fsModel_->filePath(index), true);

            QVector<int> roles = {Qt::DecorationRole};

            fsModel_->dataChanged(index, index, roles);

            fsModel_->dataChanged(index.parent(), index.parent(), roles);

        }
    });

    connect(tree_, &QTreeView::collapsed, this, [this, provider](const QModelIndex &index) {
        if (fsModel_->isDir(index)) {
            provider->setExpanded(fsModel_->filePath(index), false);

            QVector<int> roles = {Qt::DecorationRole};

            fsModel_->dataChanged(index, index, roles);

            fsModel_->dataChanged(index.parent(), index.parent(), roles);
        }
    });
    connect(tree_, &QTreeView::doubleClicked,
            this, &MainWindow::onItemDoubleClicked);

    connect(list_, &QListView::doubleClicked,
            this, &MainWindow::onItemDoubleClicked);
}
void MainWindow::onItemDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) return;

    QFileInfo info = fsModel_->fileInfo(index);
    if (info.isDir()) {
        // Navigate into folder
        QString dir = info.absoluteFilePath();
        fsModel_->setRootPath(dir);
        setRootPath(dir);
    } else {
        // For files, you can either open them or just show details
        statusBar()->showMessage("File selected: " + info.fileName());
        // Or trigger preview logic here
    }
}

void MainWindow::togglePalette(bool darkMode) {
    QPalette palette;
    QColor brandColor;
    QColor accentColor = darkMode ? QColor("#FF5722") : QColor("#2196F3");

    if (darkMode) {
        // Dark theme
        palette.setColor(QPalette::Window, QColor(53,53,53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(42,42,42));
        palette.setColor(QPalette::AlternateBase, QColor(66,66,66));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53,53,53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Highlight, QColor(140,140,140).lighter());
        palette.setColor(QPalette::HighlightedText, Qt::black);
        themeToggle->setStyleSheet(R"(
            QCheckBox { color: white; }
            QCheckBox::indicator {
                width: 12px; height: 12px; border: 1px solid black;
            }
            QCheckBox::indicator:checked {
                image: url(:/icons/icons/checkBox_checked.png);
                background-color: white;
            }
        )");
        brandColor = QColor("red");
        tb->setStyleSheet(
            "QToolBar { background:#2b2b2b; }"
            "QToolButton { color:white; }"
            );

    } else {
        // Custom light theme
        palette.setColor(QPalette::Window, QColor(245,245,245));        // soft background
        palette.setColor(QPalette::WindowText, QColor(40,40,40));       // dark text
        palette.setColor(QPalette::Base, QColor(255,255,255));          // text entry background
        palette.setColor(QPalette::Text, QColor(30,30,30));             // text color
        palette.setColor(QPalette::Highlight, QColor(100,150,255));     // selection blue
        palette.setColor(QPalette::HighlightedText, Qt::white);         // text on highlight
        palette.setColor(QPalette::Button, QColor(230,230,230));        // button background
        palette.setColor(QPalette::ButtonText, QColor(40,40,40));       // button text
        palette.setColor(QPalette::ToolTipBase, QColor(255,255,220));   // tooltip background
        palette.setColor(QPalette::ToolTipText, QColor(30,30,30));      // tooltip text
        themeToggle->setStyleSheet(R"(
            QCheckBox { color: black; }
            QCheckBox::indicator {
                width: 12px; height: 12px; border: 1px solid black;
            }
            QCheckBox::indicator:checked {
                image: url(:/icons/icons/checkBox_checked.png);
                background-color: white;
            }
        )");
        brandColor = QColor("#2196F3");
    }
    qApp->setPalette(palette);

    // brandColor = darkMode ? QColor("#FF5722") : QColor("#2196F3");
    brandColor = darkMode ? Qt::white : Qt::black;
    QSize iconSize(32,32);

    if (fileGroup) fileGroup->updateIcons(brandColor, iconSize);
    if (navGroup)  navGroup->updateIcons(brandColor, iconSize);

    auto* provider = static_cast<IconProvider*>(fsModel_->iconProvider());
    if (provider) {
        provider->setDarkMode(darkMode);
        fsModel_->dataChanged(fsModel_->index(0,0),
                              fsModel_->index(fsModel_->rowCount()-1,0));
    }

    auto* providerTb = static_cast<IconProvider*>(fsModel_->iconProvider());
    applyToolbarTheme(providerTb->darkMode());
}

void MainWindow::applyToolbarTheme(bool darkMode) {
    if (darkMode) {
        tb->setStyleSheet(
            "QToolBar { background:#2b2b2b; }"
            "QToolButton { color:white; }"
            );
        fileGroup->updateIcons(Qt::white, QSize(32,32));
        navGroup->updateIcons(Qt::white, QSize(32,32));
    } else {
        tb->setStyleSheet(
            "QToolBar { background:#fdfdfd; }"
            "QToolButton { color:black; }"
            );
        fileGroup->updateIcons(Qt::black, QSize(32,32));
        navGroup->updateIcons(Qt::black, QSize(32,32));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
