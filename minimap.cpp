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
    connect(editor_->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int value) {
                updateVisibleRegion(value, editor_->verticalScrollBar()->pageStep());
            });

}

void MiniMap::paintEvent(QPaintEvent*)
{
    if (!editor_)
        return;

    QPainter p(this);
    QColor bg = parentWidget()->palette().color(QPalette::Window);
    p.fillRect(rect(), bg);

    QFont f("Consolas");
    f.setPixelSize(5);
    p.setFont(f);
    p.setPen(QColor(200,200,200));
    p.setOpacity(0.7);

    const QStringList lines = editor_->toPlainText().split('\n');

    int y = 2;
    for (const QString& line : lines)
    {
        p.drawText(2, y, line);
        y += 4; // tiny line height
        if (y > height()) break;
    }

    // Draw visible region
    if (visibleRect_.isValid())
    {
        // Soft translucent overlay
        QColor overlay(255, 255, 255, 25);   // white, 10% opacity
        p.fillRect(visibleRect_, overlay);

        // Soft outline
        p.setPen(QColor(255, 255, 255, 80));
        p.drawRect(visibleRect_);
    }

}

void MiniMap::updateVisibleRegion(int scroll, int pageStep)
{
    if (!editor_) return;

    QScrollBar* sb = editor_->verticalScrollBar();

    double ratioStart = double(scroll) / sb->maximum();
    double ratioSize  = double(pageStep) / sb->maximum();

    int totalH = height();

    int h = qBound(10, int(ratioSize * totalH), totalH - 4);
    int y = qBound(2, int(ratioStart * totalH), totalH - h - 2);

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
