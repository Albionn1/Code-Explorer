// folderdialog.cpp
#include "folderdialog.h"
#include <QDir>
#include <QInputDialog>
#include <QHeaderView>

FolderDialog::FolderDialog(QWidget* parent)
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
    tree_->setRootIndex(model_->index(QDir::homePath()));
    tree_->setHeaderHidden(true);

    breadcrumb_ = new QLineEdit(QDir::homePath(), this);
    connect(breadcrumb_, &QLineEdit::returnPressed, this, &FolderDialog::onPathEdited);

    auto* upBtn = new QToolButton(this);
    upBtn->setIcon(QIcon(":/icons/icons/up.svg"));
    connect(upBtn, &QToolButton::clicked, this, &FolderDialog::onUp);

    auto* refreshBtn = new QToolButton(this);
    refreshBtn->setIcon(QIcon(":/icons/icons/refresh.svg"));
    connect(refreshBtn, &QToolButton::clicked, this, &FolderDialog::onRefresh);

    auto* newFolderBtn = new QToolButton(this);
    newFolderBtn->setIcon(QIcon(":/icons/icons/new_folder.svg"));
    connect(newFolderBtn, &QToolButton::clicked, this, &FolderDialog::onNewFolder);

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
        }
    });
}

// 🔹 Slots
void FolderDialog::onUp() {
    QModelIndex idx = tree_->rootIndex();
    QString path = model_->filePath(idx);
    QDir dir(path);
    dir.cdUp();
    tree_->setRootIndex(model_->index(dir.absolutePath()));
    breadcrumb_->setText(dir.absolutePath());
}

void FolderDialog::onRefresh() {
    tree_->reset();
}

void FolderDialog::onNewFolder() {
    QModelIndex idx = tree_->currentIndex();
    QString basePath = model_->filePath(idx);
    if (basePath.isEmpty()) basePath = QDir::homePath();

    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Folder name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(basePath);
        dir.mkdir(name);
        tree_->setRootIndex(model_->index(basePath));
    }
}

void FolderDialog::onPathEdited() {
    QString path = breadcrumb_->text();
    if (QDir(path).exists()) {
        tree_->setRootIndex(model_->index(path));
    }
}

void FolderDialog::setDarkMode(bool enabled) {
    if (enabled) {
        setStyleSheet(
            "QDialog { background:#2b2b2b; border-radius:8px; }"
            "QToolButton { border:none; background:transparent; padding:6px; color:white; }"
            "QLineEdit { background:#3c3c3c; color:white; border:1px solid #555; border-radius:4px; padding:4px; }"
            "QPushButton { background:#FF5722; color:white; border-radius:4px; padding:6px 12px; }"
            "QTreeView { background:#2b2b2b; color:white; }"
            );
    } else {
        setStyleSheet(
            "QDialog { background:#fdfdfd; border-radius:8px; }"
            "QToolButton { border:none; background:transparent; padding:6px; color:black; }"
            "QLineEdit { background:white; color:black; border:1px solid #ccc; border-radius:4px; padding:4px; }"
            "QPushButton { background:#2196F3; color:white; border-radius:4px; padding:6px 12px; }"
            "QTreeView { background:white; color:black; }"
            );
    }
}
