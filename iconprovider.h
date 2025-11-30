#pragma once
#include <QFileIconProvider>
#include <QSet>
#include <QString>

class IconProvider : public QFileIconProvider {
public:
    IconProvider();

    void setExpanded(const QString& path, bool expanded);
    bool isExpanded(const QString& path) const;
    void setBrandColor(const QColor& color);
    QColor brandColor() const;

    QIcon icon(const QFileInfo &info) const override;  // override for performance

private:
    QSet<QString> expandedPaths_;
    QColor brandColor_ = QColor("#2196F3");
};
