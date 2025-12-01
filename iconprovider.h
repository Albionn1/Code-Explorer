#pragma once
#include <QFileIconProvider>
#include <QSet>
#include <QString>
#include <QColor>

class IconProvider : public QFileIconProvider {
public:
    IconProvider();

    void setExpanded(const QString& path, bool expanded);
    bool isExpanded(const QString& path) const;

    void setDarkMode(bool enabled);   // NEW
    bool darkMode() const;            // NEW

    QIcon icon(const QFileInfo &info) const override;

private:
    QSet<QString> expandedPaths_;
    bool darkModeEnabled_ = false;    // NEW
};
