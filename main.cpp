#include "mainwindow.h"
#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));



    //Light palette
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(245,245,245));        // soft background
    palette.setColor(QPalette::WindowText, QColor(40,40,40));       // dark text
    palette.setColor(QPalette::Base, QColor(255,255,255));          // text entry background
    palette.setColor(QPalette::Text, QColor(30,30,30));             // text color
    palette.setColor(QPalette::Highlight, QColor(100,150,255));     // selection blue
    palette.setColor(QPalette::HighlightedText, Qt::white);         // text on highlight
    palette.setColor(QPalette::Button, QColor(230,230,230));        // button background
    palette.setColor(QPalette::ButtonText, QColor(40,40,40));       // button text
    palette.setColor(QPalette::ToolTipBase, QColor(255,255,220));   // tooltip background
    palette.setColor(QPalette::ToolTipText, QColor(30,30,30));

    //Set preferred palette
    a.setPalette(palette);

    QApplication::setApplicationName("ALBRHYTHM File Explorer");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    MainWindow w;
    w.show();
    w.resize(1500, 900); // bigger, widescreen feel
    w.setMinimumSize(1000, 700);

    return a.exec();
}
