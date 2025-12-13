#include "codeeditor.h"
#include "linenumberarea.h"

#include <QPainter>
#include <QTextBlock>

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent),
    lineNumberArea_(new LineNumberArea(this))
{
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);

    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = QString::number(blockCount()).length();
    int space = 10 + fontMetrics().horizontalAdvance('9') * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        lineNumberArea_->scroll(0, dy);
    else
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(
        QRect(cr.left(), cr.top(),
              lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea_);

    QPalette pal = palette();
    QColor bg = pal.color(QPalette::Button);
    QColor fg = pal.color(QPalette::Text);

    painter.fillRect(event->rect(), bg);
    painter.setPen(fg);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block)
                  .translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea_->width(),
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        blockNumber++;
    }
}
