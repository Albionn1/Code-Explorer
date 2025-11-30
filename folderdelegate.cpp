#include "folderdelegate.h"
#include <QPainter>
#include <QIcon>

FolderDelegate::FolderDelegate(QFileSystemModel* model, IconProvider* provider, QObject* parent)
    : QStyledItemDelegate(parent), model_(model), provider_(provider) {}

void FolderDelegate::paint(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    if (model_->isDir(index)) {
        const QString path = model_->filePath(index);
        if (provider_->isExpanded(path))
            opt.icon = QIcon(":/icons/icons/opened_folder_16.png");
        else
            opt.icon = QIcon(":/icons/icons/folder_16.png");
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
