#include "ribbongroup.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QMap>

// same tintSvgIcon helper as before
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
                         QWidget* parent)
    : QWidget(parent), actionDefs_(actions)
{
    auto* vbox = new QVBoxLayout(this);
    auto* hbox = new QHBoxLayout;

    for (const auto& act : actions) {
        auto* btn = new QToolButton;
        btn->setText(act.first);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setAutoRaise(true);
        btn->setStyleSheet("QToolButton { border:none; background:transparent; }");

        buttons_.append(btn);              // ordered list
        buttonMap_[act.first] = btn;       // lookup by name

        hbox->addWidget(btn);

        connect(btn, &QToolButton::clicked, this, [this, act] {
            emit actionTriggered(act.first);
        });
    }

    vbox->addLayout(hbox);

    auto* lbl = new QLabel(groupName);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("QLabel { font-weight:bold; color:gray; }");
    vbox->addWidget(lbl);
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
