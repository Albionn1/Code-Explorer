#include "minimap.h"
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QTextDocument>
#include <QTextBlock> // Ensure this is included

MiniMap::MiniMap(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(120);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void MiniMap::syncToEditor(QPlainTextEdit* editor)
{
    editor_ = editor;

    connect(editor_->document(), &QTextDocument::contentsChanged,
            this, [this]() {
                cacheDirty_ = true;
                update();
            });

    connect(editor_->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int value) {
                updateVisibleRegion(value);
            });

    // Connect to range changes to recalculate proportions when content height changes
    connect(editor_->verticalScrollBar(), &QScrollBar::rangeChanged,
            this, [this]() {
                updateVisibleRegion(editor_->verticalScrollBar()->value());
            });
}

void MiniMap::setHighlighter(codehighlighter* h)
{
    highlighter_ = h;
    cacheDirty_ = true;
    update();
}

void MiniMap::paintEvent(QPaintEvent*)
{
    if (!editor_) return;

    if (cacheDirty_)
        rebuildCache();

    QPainter p(this);
    p.drawPixmap(0, 0, cache_);

    if (visibleRect_.isValid())
    {
        QColor overlay(255, 255, 255, 25);
        p.fillRect(visibleRect_, overlay);

        p.setPen(QColor(255, 255, 255, 80));
        p.drawRect(visibleRect_);
    }
}

void MiniMap::updateVisibleRegion(int scroll)
{
    if (!editor_) return;

    QScrollBar* sb = editor_->verticalScrollBar();
    int max = sb->maximum();
    int min = sb->minimum();
    int pageStep = sb->pageStep();

    if (max <= min) {
        visibleRect_ = QRect();
        update();
        return;
    }

    double totalRange = double(max - min) + pageStep;
    double visibleRatio = pageStep / totalRange;

    int h = int(visibleRatio * height());

    if (h < 1) h = 1;
    if (h > height()) h = height();

    double scrollRatio = double(scroll - min) / double(max - min);

    if (scrollRatio < 0.0) scrollRatio = 0.0;
    if (scrollRatio > 1.0) scrollRatio = 1.0;

    int y = int(scrollRatio * (height() - h));

    if (y < 0) y = 0;

    if (y + h > height()) y = height() - h;

    visibleRect_ = QRect(0, y, width(), h);
    update();
}

void MiniMap::mousePressEvent(QMouseEvent* e)
{
    if (!editor_) return;

    if (visibleRect_.contains(e->pos())) {
        dragging_ = true;
        dragOffsetY_ = e->pos().y() - visibleRect_.y();
        return;
    }

    QFont f("Consolas");
    f.setPixelSize(5);
    QFontMetrics fm(f);
    int miniLineHeight = fm.height();
    int totalLines = editor_->document()->blockCount();
    if (totalLines <= 0) return;
    int virtualHeight = totalLines * miniLineHeight;

    double ratio = e->pos().y() / double(height());

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    QScrollBar* sb = editor_->verticalScrollBar();

    double scrollRange = double(sb->maximum() - sb->minimum());

    int target = int(ratio * scrollRange) + sb->minimum();

    sb->setValue(target);
}

void MiniMap::mouseMoveEvent(QMouseEvent* e)
{
    if (!editor_ || !dragging_) return;

    int newY = e->pos().y() - dragOffsetY_;
    if (newY < 0) newY = 0;

    if (newY > height() - visibleRect_.height())
        newY = height() - visibleRect_.height();

    double ratio = double(newY) / double(height() - visibleRect_.height());

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0; // The ratio must stop at 1.0 when the rect hits the bottom

    QScrollBar* sb = editor_->verticalScrollBar();

    double scrollRange = double(sb->maximum() - sb->minimum());

    int target = int(ratio * scrollRange) + sb->minimum();

    sb->setValue(target);
}
void MiniMap::mouseReleaseEvent(QMouseEvent*)
{
    dragging_ = false;
}

// *** CRITICAL FIX: Uses QTextBlock iterator for drawing ***
void MiniMap::rebuildCache()
{
    if (!editor_) return;

    QFont f("Consolas");
    f.setPixelSize(5);
    QFontMetrics fm(f);
    int miniLineHeight = fm.height();

    int totalLines = editor_->document()->blockCount();
    int virtualHeight = totalLines * miniLineHeight;

    if (virtualHeight <= 0) {
        cache_ = QPixmap(size());
        cache_.fill(Qt::transparent);
        cacheDirty_ = false;
        return;
    }

    QPixmap big(width(), virtualHeight);
    big.setDevicePixelRatio(this->devicePixelRatioF());
    big.fill(Qt::transparent);

    QPainter p(&big);
    QColor bg = parentWidget()->palette().color(QPalette::Window);
    p.fillRect(big.rect(), bg);

    p.setFont(f);
    p.setOpacity(0.7);

    QTextBlock block = editor_->document()->begin();
    int y = 0;
    while (block.isValid())
    {
        const QString line = block.text();
        int x = 2;

        if (highlighter_) {
            auto tokens = highlighter_->highlightLine(line);
            for (const MiniToken& t : tokens) {
                p.setPen(t.color);
                p.drawText(x, y, t.text);
                x += p.fontMetrics().horizontalAdvance(t.text);
            }
        } else {
            p.setPen(QColor(200, 200, 200));
            p.drawText(2, y, line);
        }

        y += miniLineHeight;
        block = block.next();
    }

    cache_ = big.scaled(width(), height(),
                        Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation);

    cacheDirty_ = false;

    QScrollBar* sb = editor_->verticalScrollBar();
    updateVisibleRegion(sb->value());
}

void MiniMap::resizeEvent(QResizeEvent*)
{
    cacheDirty_ = true;

    if (editor_) {
        QScrollBar* sb = editor_->verticalScrollBar();
        updateVisibleRegion(sb->value());
    }
}
