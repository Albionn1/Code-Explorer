
#include "iconfactory.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>

QIcon tintSvgIcon(const QString& resPath, const QColor& color, const QSize& size) {
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
