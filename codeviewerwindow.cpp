#include "codeviewerwindow.h"
#include <QFileInfo>
#include <QTabBar>
#include <qpushbutton.h>

CodeViewerWindow::CodeViewerWindow(QWidget* parent)
    : QMainWindow(parent),
    tabWidget_(new QTabWidget(this))
{
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);

    // ✅ Use palette instead of stylesheet
    QPalette pal = tabWidget_->palette();
    pal.setColor(QPalette::Window, QColor("#eaeaea"));   // tab background
    pal.setColor(QPalette::Button, QColor("#eaeaea"));   // tab button area
    pal.setColor(QPalette::Text, Qt::black);             // tab text
    tabWidget_->setPalette(pal);
    tabWidget_->setAutoFillBackground(true);

    // Close button still needs an icon
    // tabWidget_->tabBar()->setTabButton(0, QTabBar::RightSide,
    //                                    new QPushButton(QIcon(":/icons/icons/close.svg"), "", tabWidget_));
    tabWidget_->tabBar()->setStyleSheet(
        "QTabBar::close-button { "
        "   image: url(:/icons/icons/close.svg); "
        "   subcontrol-position: right; "
        "} "
        );

    connect(tabWidget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* tab = tabWidget_->widget(index);
        tabWidget_->removeTab(index);
        tab->deleteLater();
    });

    setCentralWidget(tabWidget_);
    resize(900, 700);
}

void CodeViewerWindow::openFile(const QString& path)
{
    CodeViewer* viewer = new CodeViewer(this);
    viewer->loadFile(path);

    int tabIndex = tabWidget_->addTab(viewer, QFileInfo(path).fileName());
    tabWidget_->setCurrentIndex(tabIndex);

    setWindowTitle("CodeViewer - " + QFileInfo(path).absolutePath());
}

void CodeViewerWindow::setDarkMode(bool enabled)
{
    QPalette pal = tabWidget_->palette();

    if (enabled) {
        pal.setColor(QPalette::Window, QColor(53,53,53));      // tab background
        pal.setColor(QPalette::Button, QColor(53,53,53));      // tab button area
        pal.setColor(QPalette::Base, QColor(42,42,42));        // editor background
        pal.setColor(QPalette::Text, Qt::white);               // tab text
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Highlight, QColor(140,140,140).lighter());
        pal.setColor(QPalette::HighlightedText, Qt::black);
    } else {
        pal.setColor(QPalette::Window, QColor(245,245,245));   // tab background
        pal.setColor(QPalette::Button, QColor(230,230,230));   // tab button area
        pal.setColor(QPalette::Base, QColor(255,255,255));     // editor background
        pal.setColor(QPalette::Text, QColor(30,30,30));        // tab text
        pal.setColor(QPalette::WindowText, QColor(40,40,40));
        pal.setColor(QPalette::Highlight, QColor(100,150,255));
        pal.setColor(QPalette::HighlightedText, Qt::white);
    }

    tabWidget_->setPalette(pal);
    tabWidget_->setAutoFillBackground(true);

    tabWidget_->tabBar()->setStyleSheet(
        "QTabBar::close-button { "
        "   image: url(:/icons/icons/close.svg); "
        "   subcontrol-position: right; "
        "} "
        );

    for (int i = 0; i < tabWidget_->count(); ++i) {
        if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(i))) {
            viewer->setDarkMode(enabled);
        }
    }
}
