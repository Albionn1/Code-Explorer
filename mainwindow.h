#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "codeviewerwindow.h"
#include "ribbongroup.h"
#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QTextEdit>
#include <QCheckBox>
#include <qlabel.h>
#include <QVector>
#include <QSplitter>

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

    void applyToolbarTheme(bool darkMode);

private:
    Ui::MainWindow *ui;

    QFileSystemModel* fsModel_ = nullptr;
    QTreeView* tree_ = nullptr;
    QTreeView* list_ = nullptr;
    QTextEdit* preview_ = nullptr;
    QItemSelectionModel* selModel_ = nullptr;
    QVector<int> roles = {Qt::DecorationRole};
    QLabel* headerLabel_ = nullptr;
    QAction* openAct_   = nullptr;
    QAction* renameAct_ = nullptr;
    QAction* deleteAct_ = nullptr;
    QAction* browseAct_ = nullptr;
    RibbonGroup* fileGroup = nullptr;
    RibbonGroup* navGroup  = nullptr;
    RibbonGroup* viewGroup = nullptr;
    QToolBar* tb;
    QSize iconSize_{32,32};
    QWidget* headerWidget_;
    QHBoxLayout* headerLayout_;
    QLineEdit* pathEdit_;
    QStringList history_;
    int historyIndex_ = -1;
    QSplitter* mainSplit;
    QSplitter* rightSplit;

    QAction* backAction = nullptr;
    QAction* forwardAction = nullptr;

    QVector<CodeViewerWindow*> openCodeViewerWindows_;
    CodeViewerWindow* codeViewerWindow_ = nullptr;

    QDockWidget* editorDock_ = nullptr;
    QTabWidget* editorTabs_ = nullptr;


    bool previewVisible_ = false;

    void navigateTo(const QString& dir, bool addToHistory = true);

    void setupActions();
    void setupConnections();
    void openSelected();
    void revealInExplorer();
    void deleteSelected();
    void renameSelected();
    void setRootPath(const QString& path);
    void togglePalette(bool darkMode);
    void onItemDoubleClicked(const QModelIndex& index);
    void updateDarkModeToggleUI(bool darkMode, const QSize& iconSize);
    void updateAddressBar(const QString& dir);
    void updateNavButtons();
    void onContextMenuRequested(const QPoint& pos);

};
#endif // MAINWINDOW_H
