#pragma once
#include <QPlainTextEdit>

class CodeViewer; // forward

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget* parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void updateLineNumberArea(const QRect& rect, int dy);
    void updateLineNumberAreaWidth(int);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QWidget* lineNumberArea_;
    friend class LineNumberArea;
};
