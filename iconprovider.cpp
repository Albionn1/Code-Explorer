#include "iconprovider.h"
#include "iconfactory.h"
#include <QIcon>

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

QIcon IconProvider::icon(const QFileInfo &info) const {
    if (info.isDir()) {
        QString res = isExpanded(info.absoluteFilePath())
        ? ":/icons/icons/folder_open.svg"
        : ":/icons/icons/folder.svg";
        return tintSvgIcon(res, brandColor_, QSize(16,16));
    }
    return QFileIconProvider::icon(info);
}

void IconProvider::setBrandColor(const QColor& color) {
    brandColor_ = color;
}
QColor IconProvider::brandColor() const {
    return brandColor_;
}
