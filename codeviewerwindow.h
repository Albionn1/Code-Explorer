#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include "codeviewer.h"

class CodeViewerWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CodeViewerWindow(QWidget* parent = nullptr);

    void openFile(const QString& path);
    void setDarkMode(bool enabled);

private:
    QTabWidget* tabWidget_;
};
