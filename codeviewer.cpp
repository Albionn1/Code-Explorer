#include "codeviewer.h"
#include "codeeditor.h"
#include "codehighlighter.h"

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>

CodeViewer::CodeViewer(QWidget* parent)
    : QWidget(parent),
    editor_(new CodeEditor(this)),
    highlighter_(new codehighlighter(editor_->document(), false))
{
    editor_->setReadOnly(true);
    editor_->setFont(QFont("Consolas", 11));
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor_->setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor_);
    setLayout(layout);

    setDarkMode(false);
}

void CodeViewer::loadFile(const QString& path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        editor_->setPlainText(in.readAll());
    }
}

void CodeViewer::setDarkMode(bool enabled)
{
    if (highlighter_) {
        highlighter_->setDarkMode(enabled);
    }

    QPalette p = editor_->palette();

    if (enabled) {
        p.setColor(QPalette::Base, QColor(42,42,42));        // editor background
        p.setColor(QPalette::Text, Qt::white);               // text color
        p.setColor(QPalette::Highlight, QColor(140,140,140).lighter());
        p.setColor(QPalette::HighlightedText, Qt::black);
    } else {
        p.setColor(QPalette::Base, QColor(255,255,255));     // editor background
        p.setColor(QPalette::Text, QColor(30,30,30));        // text color
        p.setColor(QPalette::Highlight, QColor(100,150,255));
        p.setColor(QPalette::HighlightedText, Qt::white);
    }

    editor_->setPalette(p);
    editor_->setAutoFillBackground(true);

    editor_->updateLineNumberAreaWidth(0);
    editor_->viewport()->update();
}

void CodeViewer::setReadOnly(bool enabled)
{
    editor_->setReadOnly(enabled);

    if (enabled) {
        editor_->setCursorWidth(0);   // hide caret
    } else {
        editor_->setCursorWidth(2);   // normal caret
    }
}
