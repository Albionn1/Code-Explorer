#pragma once
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class RibbonGroup : public QWidget {
    Q_OBJECT
public:
    RibbonGroup(const QString& groupName,
                const QVector<QPair<QString, QString>>& actions,
                QWidget* parent = nullptr,
                bool startCollapsed = false);

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
    void setCollapsed(bool collapsed);
    void setDarkMode(bool enabled);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void actionTriggered(const QString& name);

private:
    QVector<QToolButton*> buttons_;               // ordered list of buttons
    QVector<QPair<QString, QString>> actionDefs_; // action name + icon path
    QMap<QString, QToolButton*> buttonMap_;       // lookup by action name
    QList<QAction*> actions_;

    QLabel* titleLabel_;
    QToolButton* collapseButton_;
    QWidget* actionsWidget_;   // container for all buttons
    QVBoxLayout* mainLayout_;
    QPropertyAnimation* anim_;

    bool collapsed_ = true;
    bool darkMode_ = false;

};
