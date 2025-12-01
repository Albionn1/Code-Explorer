#include "iconprovider.h"
#include "iconfactory.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>

IconProvider::IconProvider() : QFileIconProvider() {}

void IconProvider::setExpanded(const QString& path, bool expanded) {
    if (expanded)
        expandedPaths_.insert(path);
    else
        expandedPaths_.remove(path);
}

bool IconProvider::isExpanded(const QString& path) const {
    return expandedPaths_.contains(path);
}

void IconProvider::setDarkMode(bool enabled) {
    darkModeEnabled_ = enabled;
}

bool IconProvider::darkMode() const {
    return darkModeEnabled_;
}

// Helper to tint SVG
static QIcon tintSvg(const QString& resPath, const QColor& color, const QSize& size) {
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

QIcon IconProvider::icon(const QFileInfo &info) const {
    if (info.isDir()) {
        QString res = isExpanded(info.absoluteFilePath())
        ? ":/icons/icons/folder-open.svg"
        : ":/icons/icons/folder.svg";

        QColor brandColor = darkModeEnabled_ ? QColor("#FF5722")   // orange for dark mode
                                             : QColor("#2196F3");  // blue for light mode
        return tintSvgIcon(res, brandColor, QSize(16,16));
    }
    return QFileIconProvider::icon(info);
}
