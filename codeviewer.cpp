#include "codeviewer.h"
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>

CodeViewer::CodeViewer(QWidget* parent)
    : QWidget(parent),
    editor_(new QPlainTextEdit(this)),
    highlighter_(new codehighlighter(editor_->document())) {

    editor_->setReadOnly(true);
    editor_->setFont(QFont("Consolas", 11));
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor_);
    setLayout(layout);
}

void CodeViewer::loadFile(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        editor_->setPlainText(in.readAll());
    }
}

void CodeViewer::setDarkMode(bool enabled) {
    if (highlighter_) {
        highlighter_->setDarkMode(enabled);
    }

    QPalette p = editor_->palette();
    if (enabled) {
        p.setColor(QPalette::Base, QColor("#202020"));
        p.setColor(QPalette::Text, QColor("#ffffff"));
    } else {
        p.setColor(QPalette::Base, QColor("#ffffff"));
        p.setColor(QPalette::Text, QColor("#000000"));
    }
    editor_->setPalette(p);
}
