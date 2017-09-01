#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QObject>
#include <QFileSystemModel>
#include <QMimeData>
#include <QDebug>
#include <QMenu>
#include <QFileIconProvider>
#include <QMimeDatabase>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStorageInfo>
#include "transferengine.h"

class FilesystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FilesystemModel(QObject *parent = nullptr);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QVariant data(const QModelIndex &index, int role) const override;
signals:

public slots:

private:
    QMimeDatabase* mimeDatabase;
};

class FilesystemDelegate : public QStyledItemDelegate
{

public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // FILESYSTEMMODEL_H
