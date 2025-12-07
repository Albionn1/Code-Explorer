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

    void updateSingleIcon(const QString& actionName,
                          const QString& iconPath,
                          const QColor& color,
                          const QSize& size,
                          const QString& newText = QString());

    QAction* findAction(const QString& name) const {
        for (QAction* act : actions_) {
            if (act->text() == name)
                return act;
        }
        return nullptr;
    }

signals:
    void actionTriggered(const QString& name);

private:
    QVector<QToolButton*> buttons_;               // ordered list of buttons
    QVector<QPair<QString, QString>> actionDefs_; // action name + icon path
    QMap<QString, QToolButton*> buttonMap_;       // lookup by action name
    QList<QAction*> actions_;

};
