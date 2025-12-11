#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include "codeeditor.h"
#pragma once
#include "linenumberarea.h"
#include <QWidget>
#include <QPlainTextEdit>
#include "codehighlighter.h"

class CodeViewer : public QWidget {
    Q_OBJECT
public:
    explicit CodeViewer(QWidget* parent = nullptr);
    void loadFile(const QString& path);
    void setDarkMode(bool enabled);
    void onTabClosed(int index);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
private:
    CodeEditor* editor_;
    codehighlighter* highlighter_;
    LineNumberArea* lineNumberArea_;

};

#endif // CODEVIEWER_H
