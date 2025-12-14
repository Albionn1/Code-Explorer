#include "codeviewerwindow.h"
#include "codeviewer.h"

#include <QFileInfo>
#include <QTabBar>
#include <QToolButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QAction>

CodeViewerWindow::CodeViewerWindow(QWidget* parent)
    : QMainWindow(parent),
    tabWidget_(new QTabWidget(this))
{
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);

    QMenuBar* menu = menuBar();

    // ----- FILE MENU -----
    QMenu* fileMenu = menu->addMenu("File");

    QAction* saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    fileMenu->addAction(saveAction);

    QAction* saveAsAction = new QAction("Save As...", this);
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    fileMenu->addAction(saveAsAction);

    // ----- EDIT MENU -----
    QMenu* editMenu = menu->addMenu("Edit");
    QAction* toggleEditAction = new QAction("Toggle Edit Mode", this);
    toggleEditAction->setShortcut(QKeySequence("Ctrl+E"));
    toggleEditAction->setCheckable(true);
    editMenu->addAction(toggleEditAction);

    QAction* undoAction = new QAction("Undo", this);
    undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    editMenu->addAction(undoAction);

    QAction* redoAction = new QAction("Redo", this);
    redoAction->setShortcut(QKeySequence("Ctrl+Y"));
    editMenu->addAction(redoAction);

    QAction* findAction = new QAction("Find", this);
    findAction->setShortcut(QKeySequence("Ctrl+F"));
    editMenu->addAction(findAction);

    // ----- VIEW MENU -----
    QMenu* viewMenu = menu->addMenu("View");

    QAction* darkModeAction = new QAction("Dark Mode", this);
    darkModeAction->setCheckable(true);
    viewMenu->addAction(darkModeAction);

    QToolBar* editorBar = new QToolBar(this);
    editorBar->setIconSize(QSize(16,16));
    editorBar->setMovable(false);
    editorBar->setFloatable(false);
    editorBar->setStyleSheet("QToolBar { border: 0; padding: 4px; }");

    // Save
    connect(saveAction, &QAction::triggered, this, [this]() {
        int index = tabWidget_->currentIndex();
        if (index >= 0) {
            if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(index))) {
                viewer->save();
            }
        }
    });

    // Save As
    connect(saveAsAction, &QAction::triggered, this, [this]() {
        int index = tabWidget_->currentIndex();
        if (index >= 0) {
            if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(index))) {
                viewer->saveAs(this);
            }
        }
    });

    // Undo
    connect(undoAction, &QAction::triggered, this, [this]() {
        if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->currentWidget()))
            viewer->editor()->undo();
    });

    // Redo
    connect(redoAction, &QAction::triggered, this, [this]() {
        if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->currentWidget()))
            viewer->editor()->redo();
    });

    // Dark mode toggle
    connect(darkModeAction, &QAction::toggled, this, [this](bool enabled) {
        setDarkMode(enabled);
    });

    // Close tab
    connect(tabWidget_, &QTabWidget::tabCloseRequested, this,
            [this](int index)
            {
                QWidget* tab = tabWidget_->widget(index);
                tabWidget_->removeTab(index);
                tab->deleteLater();
            });
    // Edit code
    connect(toggleEditAction, &QAction::toggled, this, [this](bool checked) {
        int index = tabWidget_->currentIndex();
        if (index >= 0) {
            if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(index))) {
                viewer->setReadOnly(!checked);
            }
        }
    });
    // Search bar
    connect(findAction, &QAction::triggered, this, [this]() {
        if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->currentWidget())) {
            viewer->showFindBar();
        }
    });

    QWidget* container = new QWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    layout->addWidget(editorBar);
    layout->addWidget(tabWidget_);

    setCentralWidget(container);

    setMinimumSize(1200, 900);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

void CodeViewerWindow::openFile(const QString& path)
{
    CodeViewer* viewer = new CodeViewer(this);
    viewer->loadFile(path);
    viewer->setFilePath(path);

    int tabIndex = tabWidget_->addTab(viewer, QFileInfo(path).fileName());
    tabWidget_->setCurrentIndex(tabIndex);

    setWindowTitle("CodeViewer - " + QFileInfo(path).absolutePath());
}

void CodeViewerWindow::setDarkMode(bool enabled)
{
    QPalette pal = tabWidget_->palette();

    if (enabled) {
        pal.setColor(QPalette::Window, QColor(53,53,53));
        pal.setColor(QPalette::Button, QColor(53,53,53));
        pal.setColor(QPalette::Base, QColor(42,42,42));
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Highlight, QColor(140,140,140).lighter());
        pal.setColor(QPalette::HighlightedText, Qt::black);
    } else {
        pal.setColor(QPalette::Window, QColor(245,245,245));
        pal.setColor(QPalette::Button, QColor(230,230,230));
        pal.setColor(QPalette::Base, QColor(255,255,255));
        pal.setColor(QPalette::Text, QColor(30,30,30));
        pal.setColor(QPalette::WindowText, QColor(40,40,40));
        pal.setColor(QPalette::Highlight, QColor(100,150,255));
        pal.setColor(QPalette::HighlightedText, Qt::white);
    }

    tabWidget_->setPalette(pal);
    tabWidget_->setAutoFillBackground(true);

    for (int i = 0; i < tabWidget_->count(); ++i) {
        if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(i))) {
            viewer->setDarkMode(enabled);
        }
    }
}
