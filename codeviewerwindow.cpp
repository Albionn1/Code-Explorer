#include "codeviewerwindow.h"
#include <QFileInfo>
#include <QTabBar>
#include <QPushButton>
#include <QToolButton>
#include <qboxlayout.h>
#include <qtoolbar.h>

CodeViewerWindow::CodeViewerWindow(QWidget* parent)
    : QMainWindow(parent),
    tabWidget_(new QTabWidget(this))
{
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);

    QToolBar* internalBar = new QToolBar(this);
    internalBar->setIconSize(QSize(16,16));
    internalBar->setMovable(false);
    internalBar->setFloatable(false);
    internalBar->setStyleSheet("QToolBar { border: 0; padding: 2px; }");

    QToolButton* editToggle = new QToolButton(this);
    editToggle->setText("Edit");
    editToggle->setCheckable(true);
    editToggle->setChecked(false);
    editToggle->setToolTip("Toggle edit mode");

    internalBar->addWidget(editToggle);

    editToggle->setStyleSheet("QToolButton { background:#b0b0b0; color:black; }");

    connect(editToggle, &QToolButton::toggled, this, [editToggle, this](bool checked) {
        // Update button style
        if (checked)
            editToggle->setStyleSheet("QToolButton { background:#4CAF50; color:white; }");
        else
            editToggle->setStyleSheet("QToolButton { background:#b0b0b0; color:black; }");

        // Apply to active viewer
        int index = tabWidget_->currentIndex();
        if (index >= 0) {
            if (auto* viewer = qobject_cast<CodeViewer*>(tabWidget_->widget(index))) {
                viewer->setReadOnly(!checked);
            }
        }
    });

    connect(tabWidget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* tab = tabWidget_->widget(index);
        tabWidget_->removeTab(index);
        tab->deleteLater();
    });

    QWidget* container = new QWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    layout->addWidget(internalBar);   // toolbar first
    layout->addWidget(tabWidget_);    // tabs below it

    setCentralWidget(container);

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
