#include "minimap.h"
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>

MiniMap::MiniMap(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(120); // THE WIDTH MUST BE CHANGED IN THE CONSTRUCTOR OF CODEVIEWER
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

void MiniMap::updateVisibleRegion(int scroll, int pageStep)
{
    if (!editor_) return;

    QScrollBar* sb = editor_->verticalScrollBar();

    int max = sb->maximum();
    if (max <= 0) {
        visibleRect_ = QRect();
        return;
    }

    double startRatio = double(scroll) / max;

    double sizeRatio = double(pageStep) / (max + pageStep);

    int totalH = height();

    int y = int(startRatio * totalH);
    int h = int(sizeRatio * totalH);

    h = qBound(10, h, totalH - 4);
    y = qBound(2, y, totalH - h - 2);

    visibleRect_ = QRect(0, y, width(), h);
    update();
}
void MiniMap::mousePressEvent(QMouseEvent* e)
{
    if (!editor_) return;

    if(visibleRect_.contains(e->pos())){
        dragging_ = true;
        dragOffsetY_ = e->pos().y() - visibleRect_.y();
        return;
    }

    double ratio = double(e->pos().y()) / height();
    int target = ratio * editor_->verticalScrollBar()->maximum();

    editor_->verticalScrollBar()->setValue(target);
}

void MiniMap::mouseMoveEvent(QMouseEvent* e)
{
    if (!editor_ || !dragging_) return;

    int newY = e->pos().y() - dragOffsetY_;

    newY = qBound(0, newY, height() - visibleRect_.height());

    double ratio = double(newY) / height();
    int target = ratio * editor_->verticalScrollBar()->maximum();

    editor_->verticalScrollBar()->setValue(target);
}

void MiniMap::mouseReleaseEvent(QMouseEvent*)
{
    dragging_ = false;
}

void MiniMap::rebuildCache()
{
    if (!editor_) return;

    cache_ = QPixmap(size());
    cache_.fill(Qt::transparent);

    QPainter p(&cache_);

    // Background
    QColor bg = parentWidget()->palette().color(QPalette::Window);
    p.fillRect(rect(), bg);

    // Tiny font
    QFont f("Consolas");
    f.setPixelSize(5);
    p.setFont(f);
    p.setOpacity(0.7);

    const QStringList lines = editor_->toPlainText().split('\n');

    int y = 2;
    for (const QString& line : lines)
    {
        int x = 2;

        if (highlighter_) {
            auto tokens = highlighter_->highlightLine(line);
            for (const MiniToken& t : tokens) {
                p.setPen(t.color);
                p.drawText(x, y, t.text);
                x += p.fontMetrics().horizontalAdvance(t.text);
            }
        } else {
            p.setPen(QColor(200,200,200));
            p.drawText(2, y, line);
        }

        y += 4;
        if (y > height()) break;
    }

    cacheDirty_ = false;
}
void MiniMap::resizeEvent(QResizeEvent*)
{
    cacheDirty_ = true;
}
