#include "codeviewerwindow.h"
#include <QFileInfo>

CodeViewerWindow::CodeViewerWindow(QWidget* parent)
    : QMainWindow(parent),
    tabWidget_(new QTabWidget(this)) {

    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);

    connect(tabWidget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* tab = tabWidget_->widget(index);
        tabWidget_->removeTab(index);
        tab->deleteLater();
    });

    setCentralWidget(tabWidget_);
    resize(900, 700);
}

void CodeViewerWindow::openFile(const QString& path) {
    CodeViewer* viewer = new CodeViewer(this);
    viewer->loadFile(path);

    int tabIndex = tabWidget_->addTab(viewer, QFileInfo(path).fileName());
    tabWidget_->setCurrentIndex(tabIndex);

    setWindowTitle("CodeViewer - " + QFileInfo(path).absolutePath());
}

void CodeViewerWindow::setDarkMode(bool enabled) {
    for (int i = 0; i < tabWidget_->count(); ++i) {
        CodeViewer* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(i));
        if (viewer) viewer->setDarkMode(enabled);
    }
}
