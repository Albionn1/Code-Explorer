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
    void updateVisibleRegion(int scroll);
    void setHighlighter(codehighlighter* h);
    void rebuildCache();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent*) override;
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
