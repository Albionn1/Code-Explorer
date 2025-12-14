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
    void paintEvent(QPaintEvent* event) override;
    void drawIndentGuides(QPainter* p);
    QColor indentGuideColor() const;
    int currentIndentLevel() const;
    int indentLevelOf(const QString& text) const;
    QPair<int,int> currentIndentScope() const;
    void drawIndentScope(QPainter* p);

protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QWidget* lineNumberArea_;
    friend class LineNumberArea;
};
