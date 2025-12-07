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
#include <qheaderview.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    pathEdit_ = new QLineEdit(this);
    // --- File system model ---
    fsModel_ = new QFileSystemModel(this);
    fsModel_->setRootPath(QDir::homePath());
    fsModel_->setReadOnly(false);

    // Icon provider (optional)
    auto* provider = new IconProvider();
    fsModel_->setIconProvider(provider);

    // --- Tree view ---
    tree_ = new QTreeView(this);
    tree_->setModel(fsModel_);
    tree_->setHeaderHidden(false);
    tree_->setAnimated(true);
    tree_->setExpandsOnDoubleClick(true);
    tree_->setUniformRowHeights(true);
    for (int col = 1; col < fsModel_->columnCount(); ++col) {
        tree_->hideColumn(col);
    }


    // keep provider state in sync
    connect(tree_, &QTreeView::expanded, this, [this, provider](const QModelIndex &index) {
        if (index.isValid() && fsModel_->isDir(index))
            provider->setExpanded(fsModel_->filePath(index), true);
    });
    connect(tree_, &QTreeView::collapsed, this, [this, provider](const QModelIndex &index) {
        if (index.isValid() && fsModel_->isDir(index))
            provider->setExpanded(fsModel_->filePath(index), false);
    });

    // --- List view ---
    list_ = new QTreeView(this);
    list_->setModel(fsModel_);
    list_->setRootIndex(fsModel_->index(QDir::homePath()));
    list_->setUniformRowHeights(true);
    list_->setSortingEnabled(true);
    list_->setAlternatingRowColors(false);
    list_->setColumnWidth(0, 300);
    list_->setColumnWidth(1, 120);
    list_->setColumnWidth(2, 150);
    list_->setColumnWidth(3, 180);
    // Show all columns in the main pane
    for (int col = 0; col < fsModel_->columnCount(); ++col) {
        list_->setColumnHidden(col, false);
    }


    // --- Preview ---
    preview_ = new QTextEdit(this);
    preview_->setReadOnly(true);

    // --- Main split: tree + verticalSplit ---
    rightSplit = new QSplitter(Qt::Vertical, this);
    rightSplit->addWidget(list_);
    rightSplit->addWidget(preview_);
    rightSplit->setStretchFactor(0, 1);
    rightSplit->setStretchFactor(1, 0);
    preview_->hide(); // start hidden

    // Main split: tree + rightSplit
    mainSplit = new QSplitter(Qt::Horizontal, this);
    mainSplit->addWidget(tree_);
    mainSplit->addWidget(rightSplit);
    mainSplit->setStretchFactor(0, 0);
    mainSplit->setStretchFactor(1, 1);


    // --- Central layout ---
    auto* centralLayout = new QVBoxLayout;
    centralLayout->setContentsMargins(0,0,0,0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(mainSplit);

    auto* central = new QWidget(this);
    central->setLayout(centralLayout);
    setCentralWidget(central);

    // --- Status bar ---
    statusBar()->showMessage("Ready");
    statusBar()->setStyleSheet("QStatusBar { background:#202020; color:#66ccff; }");

    // --- Initial roots ---
    const QString home = QDir::homePath();
    const QModelIndex homeIndex = fsModel_->index(home);
    tree_->setRootIndex(homeIndex);
    // list_->setRootIndex(homeIndex);

    setupActions();
    setupConnections();
    resize(1200, 800);
    setMinimumSize(1000, 700);
}

void MainWindow::setupActions() {
    tb = addToolBar("Ribbon");
    tb->setMovable(false);

    const QColor brandColor = QColor(Qt::black);

    QVector<QPair<QString, QString>> fileActions = {
        {"Open",   ":/icons/icons/open_in_new.svg"},
        {"Rename", ":/icons/icons/rename.svg"},
        {"Delete", ":/icons/icons/delete.svg"}
    };

    QVector<QPair<QString, QString>> navActions = {
        {"Back",          ":/icons/icons/arrow-left.svg"},
        {"Forward",       ":/icons/icons/arrow-right.svg"},
        {"Browse",        ":/icons/icons/folder-search.svg"},
        {"DarkModeToggle",":/icons/icons/moon-waning-crescent.svg"}
    };

    QVector<QPair<QString, QString>> viewActions = {
        {"PreviewPane", ":/icons/icons/dock-window.svg"}
    };

    fileGroup = new RibbonGroup("File Actions", fileActions, this);
    fileGroup->updateIcons(brandColor, iconSize_);

    navGroup = new RibbonGroup("Navigation", navActions, this);
    navGroup->updateIcons(brandColor, iconSize_);

    viewGroup = new RibbonGroup("View", viewActions, this);
    viewGroup->updateIcons(brandColor, iconSize_);

    // --- Back / Forward actions ---
    backAction = new QAction(
        tintSvgIcon(":/icons/icons/arrow-left.svg", brandColor, iconSize_),
        "Back", this
        );
    forwardAction = new QAction(
        tintSvgIcon(":/icons/icons/arrow-right.svg", brandColor, iconSize_),
        "Forward", this
        );
    backAction->setEnabled(false);
    forwardAction->setEnabled(false);

    connect(backAction, &QAction::triggered, this, [this]() {
        if (historyIndex_ > 0) {
            historyIndex_--;
            navigateTo(history_[historyIndex_], false);
        }
    });
    connect(forwardAction, &QAction::triggered, this, [this]() {
        if (historyIndex_ < history_.size() - 1) {
            historyIndex_++;
            navigateTo(history_[historyIndex_], false);
        }
    });

    // --- Wrap actions in tool buttons ---
    QToolButton* backButton = new QToolButton(this);
    backButton->setDefaultAction(backAction);

    QToolButton* forwardButton = new QToolButton(this);
    forwardButton->setDefaultAction(forwardAction);

    // --- Layout container ---
    QWidget* container = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(8);

    // Icons on the left
    layout->addWidget(backButton);
    layout->addWidget(forwardButton);
    layout->addWidget(fileGroup);
    layout->addWidget(navGroup);
    layout->addWidget(viewGroup);

    // Stretch before address bar
    layout->addStretch();

    // Address bar in the center
    layout->addWidget(pathEdit_);

    // Stretch after address bar
    layout->addStretch();

    container->setLayout(layout);
    tb->addWidget(container);

    // Initial dark mode state
    IconProvider* provider = static_cast<IconProvider*>(fsModel_->iconProvider());
    bool darkMode = provider ? provider->darkMode() : false;
    updateDarkModeToggleUI(darkMode, iconSize_);

    // Signals (same as before)...
    connect(fileGroup, &RibbonGroup::actionTriggered, this, [this](const QString& name){
        if (name == "Open")   openSelected();
        else if (name == "Rename") renameSelected();
        else if (name == "Delete") deleteSelected();
    });

    connect(navGroup, &RibbonGroup::actionTriggered, this, [this](const QString& name){
        if (name == "Browse") {
            IconProvider* provider = static_cast<IconProvider*>(fsModel_->iconProvider());

            QModelIndex idx = tree_->currentIndex();
            if (!idx.isValid()) idx = list_->currentIndex();

            QString currentPath;
            if (idx.isValid()) {
                QFileInfo info(fsModel_->filePath(idx));
                currentPath = info.isDir() ? info.absoluteFilePath()
                                           : info.absolutePath();
            } else {
                currentPath = QDir::homePath();
            }

            FolderDialog dlg(currentPath, this);
            if (provider) dlg.setDarkMode(provider);

            if (dlg.exec() == QDialog::Accepted) {
                QString dir = dlg.selectedPath();
                if (!dir.isEmpty()) {
                    navigateTo(dir);
                }
            }
        }
        else if (name == "DarkModeToggle") {
            IconProvider* provider = static_cast<IconProvider*>(fsModel_->iconProvider());
            if (provider) {
                bool darkMode = provider->darkMode();
                provider->setDarkMode(!darkMode);
                updateDarkModeToggleUI(!darkMode, iconSize_);
            }
        }
    });

    connect(pathEdit_, &QLineEdit::returnPressed, this, [this]() {
        QString dir = pathEdit_->text();
        navigateTo(dir);
    });

    connect(viewGroup, &RibbonGroup::actionTriggered, this,
            [this](const QString& name){
                if (name == "PreviewPane") {
                    if (!previewVisible_) {
                        preview_->show();
                        previewVisible_ = true;
                    } else {
                        preview_->hide();
                        previewVisible_ = false;
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
        // tree_->setRootIndex(dirIndex);
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
    // tree_->setRootIndex(rootIdx);
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

    connect(list_, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;

        if (fsModel_->isDir(index)) {
            QString dir = fsModel_->filePath(index);
            // This is the core fix: use navigateTo for directory changes
            navigateTo(dir);
        } else {
            // Double-click on a file
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
                list_->setRootIndex(current);
                statusBar()->showMessage(path);
                updateAddressBar(path);
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
    connect(tree_, &QTreeView::clicked, this, [this](const QModelIndex &index) {
        if (fsModel_->isDir(index)) {
            QString dir = fsModel_->filePath(index);
            // Call navigateTo() to set roots, update address bar, and manage history
            navigateTo(dir);
        }
    });

    connect(list_, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        if (fsModel_->isDir(index)) {
            QString dir = fsModel_->filePath(index);
            // Call navigateTo() to set roots, update address bar, and manage history
            navigateTo(dir);
        }
    });

}
void MainWindow::onItemDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) return;

    QFileInfo info = fsModel_->fileInfo(index);
    if (info.isDir()) {
        // Navigate into folder
        QString dir = info.absoluteFilePath();
        fsModel_->setRootPath(dir);
        setRootPath(dir);
        updateAddressBar(dir);
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
    // Determine the icon color based on the mode
    const QColor iconTint = darkMode ? Qt::white : Qt::black;
    const QSize iconSize(32, 32); // Use a consistent size

    if (darkMode) {
        tb->setStyleSheet(
            "QToolBar { background:#2b2b2b; }"
            "QToolButton { color:white; }"
            );
    } else {
        tb->setStyleSheet(
            "QToolBar { background:#fdfdfd; }"
            "QToolButton { color:black; }"
            );
    }

    fileGroup->updateIcons(iconTint, iconSize);
    navGroup->updateIcons(iconTint, iconSize);
    viewGroup->updateIcons(iconTint, iconSize);

    backAction->setIcon(
        tintSvgIcon(":/icons/icons/arrow-left.svg", iconTint, iconSize)
        );
    forwardAction->setIcon(
        tintSvgIcon(":/icons/icons/arrow-right.svg", iconTint, iconSize)
        );
}

void MainWindow::updateDarkModeToggleUI(bool darkMode, const QSize& iconSize) {
    togglePalette(darkMode);

    QString iconPath = darkMode ? ":/icons/icons/white-balance-sunny.svg"
                                : ":/icons/icons/moon-waning-crescent.svg";
    QString text     = darkMode ? "Light Mode" : "Dark Mode";
    QColor tint      = darkMode ? Qt::white : Qt::black;

    navGroup->updateSingleIcon("DarkModeToggle", iconPath, tint, iconSize, text);
}

void MainWindow::updateAddressBar(const QString& dir) {
    if (pathEdit_) pathEdit_->setText(dir);
}

void MainWindow::navigateTo(const QString& dir, bool addToHistory) {
    if (!QDir(dir).exists()) return;

    QModelIndex rootIdx = fsModel_->index(dir);
    list_->setRootIndex(rootIdx);

    updateAddressBar(dir);
    statusBar()->showMessage("Navigated to " + dir);

    if (addToHistory) {
        while (history_.size() - 1 > historyIndex_)
            history_.removeLast();

        history_.append(dir);
        historyIndex_ = history_.size() - 1;
    }

    updateNavButtons();
}
void MainWindow::updateNavButtons() {
    backAction->setEnabled(historyIndex_ > 0);
    forwardAction->setEnabled(historyIndex_ < history_.size() - 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
