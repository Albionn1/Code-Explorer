#include "ribbongroup.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QMap>
#include <QEvent>

static QIcon tintSvgIcon(const QString& resPath, const QColor& color, const QSize& size) {
    QSvgRenderer renderer(resPath);
    QPixmap base(size);
    base.fill(Qt::transparent);
    QPainter p(&base);
    renderer.render(&p);
    p.end();

    QPixmap tinted(size);
    tinted.fill(Qt::transparent);
    QPainter pt(&tinted);
    pt.setCompositionMode(QPainter::CompositionMode_Source);
    pt.fillRect(QRect(QPoint(0,0), size), color);
    pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    pt.drawPixmap(0, 0, base);
    pt.end();

    return QIcon(tinted);
}

RibbonGroup::RibbonGroup(const QString& groupName,
                         const QVector<QPair<QString, QString>>& actions,
                         QWidget* parent,
                         bool startCollapsed)
    : QWidget(parent), actionDefs_(actions), collapsed_(startCollapsed)
{
    auto* vbox = new QVBoxLayout(this);

    // --- Header row ---
    titleLabel_ = new QLabel(groupName, this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "QLabel { font-weight:bold; color:#444; padding:6px; background:#f5f5f5; border-radius:4px; }"
        "QLabel:hover { background:#e6f0fa; color:#0078d7; cursor:pointer; }"
        );

    collapseButton_ = new QToolButton(this);
    collapseButton_->setArrowType(startCollapsed ? Qt::RightArrow : Qt::DownArrow);
    collapseButton_->setAutoRaise(true);
    collapseButton_->setStyleSheet(
        "QToolButton { border:none; background:transparent; padding:4px; }"
        "QToolButton:hover { background:#e6f0fa; border-radius:4px; }"
        );

    auto* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(titleLabel_);
    headerLayout->addWidget(collapseButton_);
    vbox->addLayout(headerLayout);

    // --- Actions container ---
    actionsWidget_ = new QWidget(this);
    auto* hbox = new QHBoxLayout(actionsWidget_);
    for (const auto& act : actions) {
        auto* btn = new QToolButton;
        btn->setText(act.first);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setAutoRaise(true);
        btn->setStyleSheet("QToolButton { border:none; background:transparent; }");

        buttons_.append(btn);
        buttonMap_[act.first] = btn;
        hbox->addWidget(btn);

        connect(btn, &QToolButton::clicked, this, [this, act] {
            emit actionTriggered(act.first);
        });
    }
    vbox->addWidget(actionsWidget_);

    // --- Collapse toggle connections ---
    connect(collapseButton_, &QToolButton::clicked, this, [this](){
        setCollapsed(!collapsed_);
    });
    titleLabel_->installEventFilter(this);

    // Apply initial state
    setCollapsed(startCollapsed);
}

void RibbonGroup::updateIcons(const QColor& color, const QSize& size) {
    for (int i=0; i<buttons_.size(); ++i) {
        const auto& def = actionDefs_[i];
        buttons_[i]->setIcon(tintSvgIcon(def.second, color, size));
    }
}

void RibbonGroup::updateSingleIcon(const QString& actionName,
                                   const QString& iconPath,
                                   const QColor& color,
                                   const QSize& size,
                                   const QString& newText) {
    if (buttonMap_.contains(actionName)) {
        QToolButton* btn = buttonMap_[actionName];

        // Update icon
        btn->setIcon(tintSvgIcon(iconPath, color, size));

        // Update text if provided
        if (!newText.isEmpty()) {
            btn->setText(newText);

        }
    }

}

void RibbonGroup::setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    actionsWidget_->setVisible(!collapsed_);
    collapseButton_->setArrowType(collapsed_ ? Qt::RightArrow : Qt::DownArrow);

    if (darkMode_) {
        if (collapsed_) {
            titleLabel_->setStyleSheet(
                "QLabel { font-weight:bold; color:#dcdcdc; padding:6px; background:#2e2e2e; border-radius:4px; }"
                "QLabel:hover { background:#3a3a3a; color:#f5f5f5; cursor:pointer; }"
                );
        } else {
            titleLabel_->setStyleSheet(
                "QLabel { font-weight:bold; color:#f0f0f0; padding:6px; background:#252525; border-radius:4px; }"
                "QLabel:hover { background:#383838; cursor:pointer; }"
                );
        }
    } else {
        if (collapsed_) {
            titleLabel_->setStyleSheet(
                "QLabel { font-weight:bold; color:#111; padding:6px; background:#f0f0f0; border-radius:4px; }"
                "QLabel:hover { background:#d9d9d9; color:#000; cursor:pointer; }"
                );
        } else {
            titleLabel_->setStyleSheet(
                "QLabel { font-weight:bold; color:#000; padding:6px; background:#e0e0e0; border-radius:4px; }"
                "QLabel:hover { background:#c0c0c0; cursor:pointer; }"
                );
        }
    }
}
void RibbonGroup::setDarkMode(bool enabled) {
    darkMode_ = enabled;
    setCollapsed(collapsed_); // reapply styles with current theme
}

bool RibbonGroup::eventFilter(QObject* obj, QEvent* event) {
    if (obj == titleLabel_ && event->type() == QEvent::MouseButtonRelease) {
        setCollapsed(!collapsed_);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
