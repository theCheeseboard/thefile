#include "filesystemmodel.h"

extern TransferEngine* transferEngine;

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
            })->setEnabled(false);
            menu->addAction(QIcon::fromTheme("edit-move"), "Moving coming soon", [=] {
                TransferObject* transfer = new TransferObject(drops, info.absoluteFilePath(), TransferObject::Move);
                transferEngine->addTransfer(transfer);
            })->setEnabled(false);
            menu->exec(QCursor::pos());
        }
        return true;
    }
}

QVariant FilesystemModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DecorationRole && index.column() == 0) {
        QMimeType mime = mimeDatabase->mimeTypeForFile(fileInfo(index));
        return QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("application-octet-stream"));
    }

    return QFileSystemModel::data(index, role);
}
