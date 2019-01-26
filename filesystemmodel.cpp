#include "filesystemmodel.h"

#include <the-libs_global.h>

extern TransferEngine* transferEngine;
extern QString calculateSize(quint64 size);

FilesystemModel::FilesystemModel(QObject *parent) : QFileSystemModel(parent)
{
    mimeDatabase = new QMimeDatabase();
}

bool FilesystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (action == Qt::IgnoreAction) {
        return true;
    } else {
        if (data->hasFormat("text/uri-list")) {
            QByteArray ba = data->data("text/uri-list");

            QStringList drops = QString(ba).split("\r\n");
            QFileInfo info;
            info = fileInfo(parent);


            QMenu* menu = new QMenu();
            menu->addSection("For dragged items");
            menu->addAction(QIcon::fromTheme("edit-copy"), "Copying coming soon", [=] {
                TransferObject* transfer = new TransferObject(drops, info.absoluteFilePath(), TransferObject::Copy);
                transferEngine->addTransfer(transfer);
            });//->setEnabled(false);
            menu->addAction(QIcon::fromTheme("edit-move"), "Moving coming soon", [=] {
                TransferObject* transfer = new TransferObject(drops, info.absoluteFilePath(), TransferObject::Move);
                transferEngine->addTransfer(transfer);
            });//->setEnabled(false);
            menu->exec(QCursor::pos());
        }
        return true;
    }
}

QVariant FilesystemModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DecorationRole && index.column() == 0) {
        QMimeType mime = mimeDatabase->mimeTypeForFile(fileInfo(index));
        return QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("application-octet-stream"));
    } else if (role == Qt::DisplayRole && index.column() == 1) {
        QFileInfo info = fileInfo(index);
        if (info.isFile()) {
            return calculateSize(info.size());
        } else {
            return "";
        }
    }

    return QFileSystemModel::data(index, role);
}

void FilesystemDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex &index) const {
    QStyleOptionViewItem newOption = option;

    QFileInfo info = ((FilesystemModel*) index.model())->fileInfo(index);
    QStorageInfo sInfo(info.path());
    if (info.isHidden()) {
        newOption.palette.setColor(QPalette::WindowText, option.palette.color(QPalette::Disabled, QPalette::WindowText));
    }

    QRect iconRect;
    iconRect.setWidth(16 * theLibsGlobal::getDPIScaling());
    iconRect.setHeight(16 * theLibsGlobal::getDPIScaling());
    iconRect.moveTop(option.rect.top() + option.rect.height() / 2 - 8 * theLibsGlobal::getDPIScaling());
    iconRect.moveRight(option.rect.right());

    if (index.column() == 0) {
        newOption.rect.adjust(3 * theLibsGlobal::getDPIScaling(), 0, 0, 0);
        QStyledItemDelegate::paint(painter, newOption, index);
        painter->setPen(Qt::transparent);

        if (info.isExecutable() && !info.isDir()) {
            painter->setBrush(QColor(0, 100, 255));
        }

        if (!info.isReadable()) {
            painter->setBrush(QColor(255, 100, 0));
        }

        if (!info.isExecutable() && info.isDir()) {
            painter->setBrush(QColor(255, 100, 0));
        }

        painter->drawRect(option.rect.left(), option.rect.top(), 3 * theLibsGlobal::getDPIScaling(), option.rect.height());

        if (info.isSymLink()) {
            QPixmap px = QIcon::fromTheme("insert-link").pixmap(16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling());
            painter->drawPixmap(iconRect, px);
            iconRect.adjust(-16 * theLibsGlobal::getDPIScaling(), 0, -16 * theLibsGlobal::getDPIScaling(), 0);
        }
    } else {
        QStyledItemDelegate::paint(painter, newOption, index);
    }

    /*PaneItem item = index.data(Qt::UserRole).value<PaneItem>();
    if (item.isMounted() && item.itemType() == PaneItem::Disk) {
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect.left(), option.rect.top(), 3, option.rect.height());
    }*/
}

QSize FilesystemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}
