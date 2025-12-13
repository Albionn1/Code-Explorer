#pragma once
#include <QWidget>
#include <QPlainTextEdit>
#include "codehighlighter.h"
class MiniMap : public QWidget
{
    Q_OBJECT
public:
    explicit MiniMap(QWidget* parent = nullptr);

    void syncToEditor(QPlainTextEdit* editor);
    void updateVisibleRegion(int scroll, int pageStep);
    void setHighlighter(codehighlighter* h) { highlighter_ = h; }
    void rebuildCache();
    void resizeEvent(QResizeEvent*);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    QPlainTextEdit* editor_ = nullptr;
    QRect visibleRect_;
    bool dragging_ = false;
    int dragOffsetY_ = 0;
    codehighlighter* highlighter_ = nullptr;
    QPixmap cache_;
    bool cacheDirty_ = true;

};
