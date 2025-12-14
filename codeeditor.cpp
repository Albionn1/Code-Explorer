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
    int padding = charWidth * 0.6;

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

int CodeEditor::indentLevelOf(const QString& text, int* pixelX) const
{
    int spaces = 0;
    int i = 0;

    for (; i < text.size(); ++i) {
        QChar c = text[i];
        if (c == ' ') spaces++;
        else if (c == '\t') spaces += 4;
        else break;
    }

    if (pixelX) {
        int cw = fontMetrics().horizontalAdvance(' ');
        *pixelX = spaces * cw;
    }

    return spaces / 4;
}

QPair<int,int> CodeEditor::currentIndentScope() const
{
    QTextBlock block = textCursor().block();
    int start = block.blockNumber();

    int dummyX = 0;
    int baseIndent = indentLevelOf(block.text(), &dummyX);

    QTextBlock b = block.next();
    int end = start;

    while (b.isValid()) {
        int ignoreX = 0;
        int level = indentLevelOf(b.text(), &ignoreX);

        if (level < baseIndent && !b.text().trimmed().isEmpty())
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

    QColor scopeColor = (brightness > 128)
                            ? QColor(0, 0, 0, 12)
                            : QColor(255, 255, 255, 18);

    p->save();
    p->setBrush(scopeColor);
    p->setPen(Qt::NoPen);

    QTextBlock block = document()->findBlockByNumber(start);

    int firstIndentX = 0;
    {
        int tmp = 0;
        indentLevelOf(block.text(), &tmp);
        firstIndentX = tmp;
    }

    int paddingLeft  = firstIndentX - fontMetrics().horizontalAdvance(' ') * 0.5;
    int paddingRight = firstIndentX + fontMetrics().horizontalAdvance(' ') * 8; // extend a bit into text

    while (block.isValid() && block.blockNumber() <= end)
    {
        QRectF r = blockBoundingGeometry(block).translated(contentOffset());

        r.setLeft(qMax(0, paddingLeft));
        r.setRight(qMin(viewport()->width(), paddingRight));

        p->drawRect(r);

        block = block.next();
    }

    p->restore();
}

QPair<int,int> CodeEditor::braceScope(QTextCursor cursor) const
{
    QTextDocument* doc = document();

    QTextCursor scan = cursor;
    int openPos = -1;

    {
        QTextCursor c = scan;
        while (!c.atStart()) {
            c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            QString ch = c.selectedText();
            if (ch.isEmpty())
                break;

            if (ch[0] == '{') {
                openPos = c.position() - 1;
                break;
            }
        }
    }

    if (openPos < 0) {
        QTextCursor c = scan;
        while (!c.atEnd()) {
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            QString ch = c.selectedText();
            if (ch.isEmpty())
                break;

            if (ch[0] == '{') {
                openPos = c.position() - 1;
                break;
            }
        }
    }

    if (openPos < 0)
        return {-1, -1};

    QTextCursor braceCursor(doc);
    braceCursor.setPosition(openPos);

    int depth = 0;
    int closePos = -1;

    while (!braceCursor.atEnd()) {
        braceCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        QString ch = braceCursor.selectedText();
        if (ch.isEmpty())
            break;

        if (ch[0] == '{') {
            depth++;
        } else if (ch[0] == '}') {
            depth--;
            if (depth == 0) {
                closePos = braceCursor.position() - 1;
                break;
            }
        }
    }

    if (closePos < 0)
        return {-1, -1};

    QTextCursor openBlockCursor(doc);
    openBlockCursor.setPosition(openPos);

    QTextCursor closeBlockCursor(doc);
    closeBlockCursor.setPosition(closePos);

    int startBlock = openBlockCursor.block().blockNumber();
    int endBlock   = closeBlockCursor.block().blockNumber();

    int cursorBlock = cursor.block().blockNumber();
    if (cursorBlock < startBlock || cursorBlock > endBlock)
        return {-1, -1};

    return { startBlock, endBlock };
}


QPair<int,int> CodeEditor::indentScope() const
{
    QTextBlock block = textCursor().block();
    int start = block.blockNumber();

    int dummy = 0;
    int baseIndent = indentLevelOf(block.text(), &dummy);

    QTextBlock b = block.next();
    int end = start;

    while (b.isValid()) {
        int px = 0;
        int level = indentLevelOf(b.text(), &px);

        if (level < baseIndent && !b.text().trimmed().isEmpty())
            break;

        end = b.blockNumber();
        b = b.next();
    }

    return {start, end};
}

QPair<int,int> CodeEditor::unifiedScope() const
{
    QTextCursor c = textCursor();

    auto chain = ifElseChainScope();
    if (chain.first != -1 && chain.second != -1 && chain.first != chain.second)
        return chain;

    auto brace = braceScope(c);
    if (brace.first != -1 && brace.second != -1)
        return brace;

    return {-1, -1};
}


void CodeEditor::drawScope(QPainter* p)
{
    auto scope = unifiedScope();
    int start = scope.first;
    int end   = scope.second;

    if (start < 0 || end < 0 || start == end)
        return;

    QColor base = palette().color(QPalette::Base);
    int brightness = qGray(base.rgb());

    QColor scopeColor = (brightness > 128)
                            ? QColor(0, 0, 0, 12)
                            : QColor(255, 255, 255, 18);

    p->save();
    p->setBrush(scopeColor);
    p->setPen(Qt::NoPen);

    QTextBlock block = document()->findBlockByNumber(start);

    while (block.isValid() && block.blockNumber() <= end)
    {
        QRectF r = blockBoundingGeometry(block).translated(contentOffset());
        r.setLeft(0);
        r.setRight(viewport()->width());
        p->drawRect(r);

        block = block.next();
    }

    p->restore();
}

bool CodeEditor::isIfElseLine(const QString& text) const
{
    QString t = text.trimmed();
    return t.startsWith("if ") ||
           t.startsWith("if(") ||
           t.startsWith("else if") ||
           t.startsWith("else");
}

QPair<int,int> CodeEditor::ifElseChainScope() const
{
    QTextBlock block = textCursor().block();
    QString current = block.text().trimmed();
    int line = block.blockNumber();

    if (!isIfElseLine(current))
        return {-1, -1};

    QTextBlock up = block;
    int start = line;

    while (up.isValid()) {
        QString t = up.text().trimmed();
        if (!isIfElseLine(t))
            break;
        start = up.blockNumber();
        up = up.previous();
    }

    QTextBlock down = block;
    int end = line;

    while (down.isValid()) {
        QString t = down.text().trimmed();
        if (!isIfElseLine(t))
            break;
        end = down.blockNumber();
        down = down.next();
    }

    int finalStart = start;
    int finalEnd   = end;

    for (int i = start; i <= end; ++i) {
        QTextBlock b = document()->findBlockByNumber(i);
        QString t = b.text().trimmed();

        if (!isIfElseLine(t))
            continue;

        QTextCursor c(b);
        auto br = braceScope(c);

        if (br.first != -1) {
            finalStart = qMin(finalStart, br.first);
            finalEnd   = qMax(finalEnd, br.second);
        }
    }

    return {finalStart, finalEnd};
}


void CodeEditor::paintEvent(QPaintEvent* event)
{
    QPainter p(viewport());

    // drawScope(&p); //it doesn't work as intended
    drawIndentGuides(&p);
    QPlainTextEdit::paintEvent(event);
}

