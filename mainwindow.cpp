#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "iconfactory.h"
#include "iconprovider.h"
#include "folderdelegate.h"

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
    auto* tb = addToolBar("Actions");
    tb->setIconSize(QSize(24,24));
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);

    auto* openAct   = tb->addAction("Open");
    auto* revealAct = tb->addAction("Reveal");
    auto* renameAct = tb->addAction("Rename");
    auto* deleteAct = tb->addAction("Delete");
    auto* browseAct = tb->addAction("Browse…");

    // Connect once in your constructor
    connect(themeToggle, &QCheckBox::toggled,
            this, &MainWindow::togglePalette);

    connect(openAct,   &QAction::triggered, this, &MainWindow::openSelected);
    connect(revealAct, &QAction::triggered, this, &MainWindow::revealInExplorer);
    connect(renameAct, &QAction::triggered, this, &MainWindow::renameSelected);
    connect(deleteAct, &QAction::triggered, this, &MainWindow::deleteSelected);
    connect(browseAct, &QAction::triggered, this, [this]{
        const auto dir = QFileDialog::getExistingDirectory(this, "Choose root", fsModel_->rootPath());
        if (!dir.isEmpty()) setRootPath(dir);
    });

    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Browse…", [this]{
        const auto dir = QFileDialog::getExistingDirectory(this, "Choose root", fsModel_->rootPath());
        if (!dir.isEmpty()) setRootPath(dir);
    });
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", [this]{ close(); });
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
}

void MainWindow::togglePalette(bool checked) {
    darkModeEnabled = checked;

    // Pick accent color
    QColor brandColor = darkModeEnabled ? QColor("#FF5722")   // Deep Orange
                                        : QColor("#2196F3");  // Blue

    QSize iconSize(24,24);

    // Re‑generate tinted icons
    auto openIcon   = tintSvgIcon(":/icons/open_in_new.svg", brandColor, iconSize);
    auto revealIcon = tintSvgIcon(":/icons/file_find.svg",   brandColor, iconSize);
    auto renameIcon = tintSvgIcon(":/icons/edit.svg",        brandColor, iconSize);
    auto deleteIcon = tintSvgIcon(":/icons/delete.svg",      brandColor, iconSize);
    auto browseIcon = tintSvgIcon(":/icons/folder_open.svg", brandColor, iconSize);

    // Update toolbar actions (assuming you added them in order)
    auto* tb = findChild<QToolBar*>("Actions");
    if (tb && tb->actions().size() >= 5) {
        tb->actions()[0]->setIcon(openIcon);
        tb->actions()[1]->setIcon(revealIcon);
        tb->actions()[2]->setIcon(renameIcon);
        tb->actions()[3]->setIcon(deleteIcon);
        tb->actions()[4]->setIcon(browseIcon);
    }

    // Update folder icons via IconProvider
    auto* provider = static_cast<IconProvider*>(fsModel_->iconProvider());
    if (provider) {
        provider->setBrandColor(brandColor);
        // Force redraw of the whole model
        fsModel_->dataChanged(fsModel_->index(0,0),
                              fsModel_->index(fsModel_->rowCount()-1,0));
    }

    // Optional: background
    if (darkModeEnabled)
        qApp->setStyleSheet("QMainWindow { background-color: #121212; }");
    else
        qApp->setStyleSheet("QMainWindow { background-color: #FAFAFA; }");
}
MainWindow::~MainWindow()
{
    delete ui;
}
