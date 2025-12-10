#ifndef CODEVIEWER_H
#define CODEVIEWER_H
#pragma once
#include <QWidget>
#include <QPlainTextEdit>
#include "codehighlighter.h"

class CodeViewer : public QWidget {
    Q_OBJECT
public:
    explicit CodeViewer(QWidget* parent = nullptr);
    void loadFile(const QString& path);
    void setDarkMode(bool enabled);

private:
    QPlainTextEdit* editor_;
    codehighlighter* highlighter_;
};

#endif // CODEVIEWER_H
