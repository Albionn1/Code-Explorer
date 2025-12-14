#pragma once
#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include "minimap.h"
#include "codeeditor.h"
#include "linenumberarea.h"
#include <QWidget>
#include <QPlainTextEdit>
#include "codehighlighter.h"
#include <QLineEdit>
#include <QLabel>

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
    void replaceOne();
    void replaceAll();
    int indentLevel(const QString& line) const;

private:
    CodeEditor* editor_;
    codehighlighter* highlighter_;
    LineNumberArea* lineNumberArea_;
    QString filePath_;
    QWidget* findbar_ = nullptr;
    QLineEdit* findField_ = nullptr;
    bool regexEnabled_ = false;
    bool caseSensitive_ = false;
    void updateHighlights();
    QLabel* matchCountLabel_ = nullptr;
    QLineEdit* replaceField_ = nullptr;
    QWidget* replaceBar_ = nullptr;
    MiniMap* minimap_ = nullptr;

};

#endif // CODEVIEWER_H
