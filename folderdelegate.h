#pragma once
#include <QStyledItemDelegate>
#include <QFileSystemModel>
#include "iconprovider.h"

class FolderDelegate : public QStyledItemDelegate {
public:
    FolderDelegate(QFileSystemModel* model, IconProvider* provider, QObject* parent = nullptr);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    QFileSystemModel* model_;
    IconProvider* provider_;
};
