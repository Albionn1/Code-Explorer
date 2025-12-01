#pragma once
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class RibbonGroup : public QWidget {
    Q_OBJECT
public:
    RibbonGroup(const QString& groupName,
                const QVector<QPair<QString, QString>>& actions,
                QWidget* parent = nullptr);

    void updateIcons(const QColor& color, const QSize& size);

signals:
    void actionTriggered(const QString& name);

private:
    QVector<QToolButton*> buttons_;
    QVector<QPair<QString, QString>> actionDefs_;
};
