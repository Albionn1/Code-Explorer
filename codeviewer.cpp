#include "codeviewer.h"
#include "codeeditor.h"
#include "codehighlighter.h"

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QFileDialog>
#include <qtoolbutton.h>

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

    // --- FIND BAR ---
    findbar_ = new QWidget(this);
    findbar_->setVisible(false);

    QHBoxLayout* findLayout = new QHBoxLayout(findbar_);
    findLayout->setContentsMargins(4, 4, 4, 4);

    findField_ = new QLineEdit(findbar_);
    findField_->setPlaceholderText("Find...");
    findLayout->addWidget(findField_);

    QToolButton* nextBtn = new QToolButton(findbar_);
    nextBtn->setText("Next");
    findLayout->addWidget(nextBtn);

    QToolButton* prevBtn = new QToolButton(findbar_);
    prevBtn->setText("Prev");
    findLayout->addWidget(prevBtn);

    QToolButton* closeBtn = new QToolButton(findbar_);
    closeBtn->setText("X");
    findLayout->addWidget(closeBtn);

    // Connect buttons
    connect(nextBtn, &QToolButton::clicked, this, &CodeViewer::findNext);
    connect(prevBtn, &QToolButton::clicked, this, &CodeViewer::findPrevious);
    connect(closeBtn, &QToolButton::clicked, this, &CodeViewer::hideFindBar);

    // Add find bar to layout ABOVE the editor
    layout->insertWidget(0, findbar_);

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

bool CodeViewer::save()
{
    if (filePath_.isEmpty())
        return false; // should call Save As instead

    QFile file(filePath_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << editor_->toPlainText();
    return true;
}

bool CodeViewer::saveAs(QWidget* parent)
{
    QString newPath = QFileDialog::getSaveFileName(parent, "Save As", filePath_);
    if (newPath.isEmpty())
        return false;

    filePath_ = newPath;
    return save();
}

void CodeViewer::showFindBar()
{
    findbar_->setVisible(true);
    findField_->setFocus();
    findField_->selectAll();
}

void CodeViewer::hideFindBar()
{
    findbar_->setVisible(false);
    editor_->setFocus();
}

void CodeViewer::findNext()
{
    QString text = findField_->text();
    if (text.isEmpty()) return;

    editor_->find(text);
}

void CodeViewer::findPrevious()
{
    QString text = findField_->text();
    if (text.isEmpty()) return;

    editor_->find(text, QTextDocument::FindBackward);
}
