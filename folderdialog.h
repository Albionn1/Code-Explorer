#pragma once
#include "iconprovider.h"
#include <QDialog>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

class FolderDialog : public QDialog {
    Q_OBJECT
public:
    explicit FolderDialog(const QString& startPath, QWidget* parent = nullptr);

    QString selectedPath() const { return selectedPath_; }

    void setDarkMode(bool enabled);
    void setDarkMode(IconProvider* provider);
private slots:
    void onUp();
    void onRefresh();
    void onNewFolder();
    void onPathEdited();


private:
    QFileSystemModel* model_;
    QTreeView* tree_;
    QLineEdit* breadcrumb_;
    QString selectedPath_;
    QToolButton* upBtn;
    QToolButton* refreshBtn;
    QToolButton* newFolderBtn;
};
