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

void CodeEditor::drawIndentGuides(QPainter* p)
{
    p->save();
    p->setPen(indentGuideColor());

    int charWidth = fontMetrics().horizontalAdvance(' ');
    int indentWidth = 4 * charWidth;
    int padding = charWidth * 0.6; // tweak to taste

    QTextBlock block = firstVisibleBlock();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();

    while (block.isValid() && top <= viewport()->height())
    {
        QString text = block.text();

        int spaces = 0;
        for (QChar c : text) {
            if (c == ' ') spaces++;
            else if (c == '\t') spaces += 4;
            else break;
        }

        int indentLevels = spaces / 4;
        int blockHeight = blockBoundingRect(block).height();
        int bottom = top + blockHeight;

        int activeLevel = currentIndentLevel();

        for (int level = 1; level <= indentLevels; ++level)
        {
            int x = level * indentWidth - indentWidth / 2 - padding;

            if (level == activeLevel) {

                QColor active = indentGuideColor();
                active.setAlpha(active.alpha() + 40); // boost visibility
                p->setPen(active);
            } else {
                p->setPen(indentGuideColor());
            }

            p->drawLine(x, top, x, bottom);
        }

        block = block.next();
        top = bottom;
    }

    p->restore();
}

QColor CodeEditor::indentGuideColor() const
{
    QColor base = palette().color(QPalette::Base);
    int brightness = qGray(base.rgb());

    if (brightness > 128) {
        // Light mode → darker guides
        return QColor(0, 0, 0, 70);
    } else {
        // Dark mode → lighter guides
        return QColor(255, 255, 255, 40);
    }
}

int CodeEditor::currentIndentLevel() const
{
    QTextBlock block = textCursor().block();
    QString text = block.text();

    int spaces = 0;
    for(QChar c : text){
        if(c == ' ')spaces++;
        else if(c == '\t')spaces += 4;
        else break;
    }
    return spaces / 4;
}

int CodeEditor::indentLevelOf(const QString& text) const
{
    int spaces = 0;
    for (QChar c : text) {
        if (c == ' ') spaces++;
        else if (c == '\t') spaces += 4;
        else break;
    }
    return spaces / 4;
}

QPair<int,int> CodeEditor::currentIndentScope() const
{
    QTextBlock block = textCursor().block();
    int start = block.blockNumber();

    int baseIndent = indentLevelOf(block.text());
    if (baseIndent == 0)
        return {start, start};

    // Walk downward until indentation drops
    QTextBlock b = block.next();
    int end = start;

    while (b.isValid()) {
        int level = indentLevelOf(b.text());
        if (level < baseIndent)
            break;
        end = b.blockNumber();
        b = b.next();
    }

    return {start, end};
}

void CodeEditor::drawIndentScope(QPainter* p)
{
    auto scope = currentIndentScope();
    int start = scope.first;
    int end   = scope.second;

    if (start == end)
        return;

    QColor base = palette().color(QPalette::Base);
    int brightness = qGray(base.rgb());

    QColor scopeColor;
    if (brightness > 128) {
        // Light mode
        scopeColor = QColor(0, 0, 0, 12);
    } else {
        // Dark mode
        scopeColor = QColor(255, 255, 255, 18);
    }

    p->save();
    p->setBrush(scopeColor);
    p->setPen(Qt::NoPen);

    QTextBlock block = document()->findBlockByNumber(start);

    while (block.isValid() && block.blockNumber() <= end)
    {
        QRectF r = blockBoundingGeometry(block)
        .translated(contentOffset());

        r.setLeft(0);                 // full width
        r.setRight(viewport()->width());

        p->drawRect(r);

        block = block.next();
    }

    p->restore();
}

void CodeEditor::paintEvent(QPaintEvent* event)
{

    QPainter p(viewport());
    drawIndentScope(&p);
    drawIndentGuides(&p);

    QPlainTextEdit::paintEvent(event);
}
