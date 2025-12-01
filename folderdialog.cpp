// folderdialog.cpp
#include "folderdialog.h"
#include "iconprovider.h"
#include "iconfactory.h"
#include <QDir>
#include <QInputDialog>
#include <QHeaderView>
#include <QDebug> // Added for potential debugging
#include <QMessageBox>

FolderDialog::FolderDialog(const QString& startPath, QWidget* parent)
    : QDialog(parent), model_(new QFileSystemModel(this)), tree_(new QTreeView(this))
{
    setWindowTitle("Select Folder");
    setMinimumSize(600, 400);
    setStyleSheet("QDialog { background: #fdfdfd; border-radius: 8px; }"
                  "QToolButton { border:none; background:transparent; padding:6px; }"
                  "QLineEdit { border:1px solid #ccc; border-radius:4px; padding:4px; }");

    model_->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    model_->setRootPath(QDir::rootPath());

    tree_->setModel(model_);
    tree_->setHeaderHidden(true);

    QString initialPath = QDir(startPath).exists() ? startPath : QDir::homePath();

    QDir initialDir(initialPath);
    initialDir.cdUp();
    QModelIndex rootIdx = model_->index(initialDir.absolutePath());
    tree_->setRootIndex(rootIdx);

    QModelIndex currentIdx = model_->index(initialPath);
    if (currentIdx.isValid()) {
        tree_->setCurrentIndex(currentIdx);
        tree_->scrollTo(currentIdx);
    }

    breadcrumb_ = new QLineEdit(initialPath, this);

    connect(breadcrumb_, &QLineEdit::returnPressed, this, &FolderDialog::onPathEdited);

    QSize iconSize(24,24);
    QColor iconColor = Qt::black; // default light mode

    upBtn = new QToolButton(this);
    upBtn->setIcon(tintSvgIcon(":/icons/icons/up.svg", iconColor, iconSize));

    refreshBtn = new QToolButton(this);
    refreshBtn->setIcon(tintSvgIcon(":/icons/icons/refresh.svg", iconColor, iconSize));

    newFolderBtn = new QToolButton(this);
    newFolderBtn->setIcon(tintSvgIcon(":/icons/icons/new_folder.svg", iconColor, iconSize));

    connect(refreshBtn, &QToolButton::clicked, this, &FolderDialog::onRefresh);
    connect(newFolderBtn, &QToolButton::clicked, this, &FolderDialog::onNewFolder);
    connect(upBtn, &QToolButton::clicked, this, &FolderDialog::onUp);

    auto* navBar = new QHBoxLayout;
    navBar->addWidget(upBtn);
    navBar->addWidget(refreshBtn);
    navBar->addWidget(newFolderBtn);
    navBar->addWidget(breadcrumb_);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(navBar);
    layout->addWidget(tree_);

    auto* okBtn = new QPushButton("Select", this);
    okBtn->setStyleSheet("QPushButton { background:#2196F3; color:white; border-radius:4px; padding:6px 12px; }");
    layout->addWidget(okBtn, 0, Qt::AlignRight);

    connect(okBtn, &QPushButton::clicked, this, [this] {
        QModelIndex idx = tree_->currentIndex();

        if (idx.isValid()) {
            selectedPath_ = model_->filePath(idx);
            accept();
        } else {
            QString pathFromBreadcrumb = breadcrumb_->text();
            if (QDir(pathFromBreadcrumb).exists()) {
                selectedPath_ = pathFromBreadcrumb;
                accept();
            } else {
                QMessageBox::warning(this, "No Path Selected", "Please select a valid folder or enter a valid path.");
            }
        }
    });

    // Connect the tree view selection change to update the breadcrumb
    connect(tree_->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &/*previous*/) {
                if (current.isValid()) {
                    QString path = model_->filePath(current);
                    breadcrumb_->setText(path);
                }
            });
}

void FolderDialog::onUp() {
    QModelIndex idx = tree_->rootIndex();
    QString path = model_->filePath(idx);

    QDir dir(path);
    if (!dir.isRoot()) { // Prevents going 'up' from the file system root
        dir.cdUp();
    }

    QString parentPath = dir.absolutePath();
    tree_->setRootIndex(model_->index(parentPath));
    breadcrumb_->setText(parentPath);

    QModelIndex newCurrentIdx = model_->index(path);
    if (newCurrentIdx.isValid()) {
        tree_->setCurrentIndex(newCurrentIdx);
    }
}

void FolderDialog::onRefresh() {

    QModelIndex rootIndex = tree_->rootIndex();
    QString currentPath = model_->filePath(rootIndex);

    model_->index(currentPath);

    tree_->reset();
}

void FolderDialog::onNewFolder() {
    QModelIndex idx = tree_->currentIndex();
    QString basePath = model_->filePath(idx);

    // Ensure we have a valid path to create the folder in
    if (!QDir(basePath).exists()) {
        basePath = model_->filePath(tree_->rootIndex());
        if (!QDir(basePath).exists()) {
            basePath = QDir::homePath();
        }
    }

    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Folder name:", QLineEdit::Normal, "", &ok);

    if (ok && !name.isEmpty()) {
        QDir dir(basePath);
        if (dir.mkdir(name)) {
            QString newFolderPath = QDir::cleanPath(basePath + QDir::separator() + name);
            // Navigate to and select the newly created folder
            QModelIndex newFolderIdx = model_->index(newFolderPath);
            tree_->setRootIndex(model_->index(basePath));
            if (newFolderIdx.isValid()) {
                tree_->setCurrentIndex(newFolderIdx);
                breadcrumb_->setText(newFolderPath);
            }
        }
    }
}

void FolderDialog::onPathEdited() {
    QString path = breadcrumb_->text();
    if (QDir(path).exists()) {
        // When path is edited, set the parent as the root and select the folder itself
        QDir dir(path);
        QModelIndex currentIdx = model_->index(path);

        dir.cdUp();
        tree_->setRootIndex(model_->index(dir.absolutePath()));

        if (currentIdx.isValid()) {
            tree_->setCurrentIndex(currentIdx);
        }
    } else {
        // Path is invalid, revert breadcrumb to current view's path
        breadcrumb_->setText(model_->filePath(tree_->currentIndex().isValid() ? tree_->currentIndex() : tree_->rootIndex()));
    }
}

void FolderDialog::setDarkMode(IconProvider* provider) {
    bool enabled = provider->darkMode();   // use your flag
    QSize iconSize(24,24);
    QColor iconColor = enabled ? Qt::white : Qt::black;
    QColor accentColor = enabled ? QColor("#FF5722") : QColor("#2196F3");
    QString accent = accentColor.name();

    upBtn->setIcon(tintSvgIcon(":/icons/icons/arrow-up-bold.svg", iconColor, iconSize));
    refreshBtn->setIcon(tintSvgIcon(":/icons/icons/refresh.svg", iconColor, iconSize));
    newFolderBtn->setIcon(tintSvgIcon(":/icons/icons/new-folder.svg", iconColor, iconSize));

    if (enabled) {
        setStyleSheet(
            "QDialog { background:#2b2b2b; border-radius:8px; }"
            "QToolButton { border:none; background:transparent; padding:6px; color:white; }"
            "QLineEdit { background:#3c3c3c; color:white; border:1px solid #555; border-radius:4px; padding:4px; }"
            "QPushButton { background:" + accent + "; color:white; border-radius:4px; padding:6px 12px; }"
                       "QTreeView { background:#2b2b2b; color:white; }"
            );
    } else {
        setStyleSheet(
            "QDialog { background:#fdfdfd; border-radius:8px; }"
            "QToolButton { border:none; background:transparent; padding:6px; color:black; }"
            "QLineEdit { background:white; color:black; border:1px solid #ccc; border-radius:4px; padding:4px; }"
            "QPushButton { background:" + accent + "; color:white; border-radius:4px; padding:6px 12px; }"
                       "QTreeView { background:white; color:black; }"
            );
    }
}
