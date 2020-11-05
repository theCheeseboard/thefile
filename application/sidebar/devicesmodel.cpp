/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "devicesmodel.h"

#include <QAbstractItemView>
#include <QPainter>
#include <driveobjectmanager.h>
#include <DriveObjects/filesysteminterface.h>
#include <DriveObjects/diskobject.h>

struct DevicesModelPrivate {
    QList<DiskObject*> filesystemDisks;
};

DevicesModel::DevicesModel(QObject* parent)
    : QAbstractListModel(parent) {
    d = new DevicesModelPrivate();
    connect(DriveObjectManager::instance(), &DriveObjectManager::filesystemDisksChanged, this, &DevicesModel::updateDisks);
    updateDisks();
}

DevicesModel::~DevicesModel() {
    delete d;
}

int DevicesModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->filesystemDisks.count();
}

QVariant DevicesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    DiskObject* diskObject = d->filesystemDisks.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return diskObject->displayName();
        case Qt::DecorationRole:
            return diskObject->icon();
        case DiskObjectRole:
            return QVariant::fromValue(diskObject);
        case MountedRole: {
            FilesystemInterface* fs = diskObject->interface<FilesystemInterface>();
            if (fs && !fs->mountPoints().isEmpty()) return true;
            return false;
        }
    }

    return QVariant();
}

void DevicesModel::updateDisks() {
    beginResetModel();
    d->filesystemDisks = DriveObjectManager::filesystemDisks();
    endResetModel();
}
