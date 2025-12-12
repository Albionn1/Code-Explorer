#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include "codeeditor.h"
#pragma once
#include "linenumberarea.h"
#include <QWidget>
#include <QPlainTextEdit>
#include "codehighlighter.h"
#include <QLineEdit>

class CodeViewer : public QWidget {
    Q_OBJECT
public:
    explicit CodeViewer(QWidget* parent = nullptr);
    void loadFile(const QString& path);
    void setDarkMode(bool enabled);
    void onTabClosed(int index);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setReadOnly(bool enabled);
    QString filePath() const { return filePath_; }
    void setFilePath(const QString& path) { filePath_ = path; }
    bool save();
    bool saveAs(QWidget* parent);
    CodeEditor* editor() const { return editor_; }
    void showFindBar();
    void hideFindBar();
    void findNext();
    void findPrevious();

private:
    CodeEditor* editor_;
    codehighlighter* highlighter_;
    LineNumberArea* lineNumberArea_;
    QString filePath_;
    QWidget* findbar_ = nullptr;
    QLineEdit* findField_ = nullptr;

};

#endif // CODEVIEWER_H
