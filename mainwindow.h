#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QTextEdit>
#include <QCheckBox>
#include <qlabel.h>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QFileSystemModel* fsModel_ = nullptr;
    QTreeView* tree_ = nullptr;
    QListView* list_ = nullptr;
    QTextEdit* preview_ = nullptr;
    QCheckBox* themeToggle;
    QItemSelectionModel* selModel_ = nullptr;
    QVector<int> roles = {Qt::DecorationRole};
    QLabel* headerLabel_ = nullptr;
    bool darkModeEnabled = false;

    void setupActions();
    void setupConnections();
    void openSelected();
    void revealInExplorer();
    void deleteSelected();
    void renameSelected();
    void setRootPath(const QString& path);
    void togglePalette(bool checked);
};
#endif // MAINWINDOW_H
