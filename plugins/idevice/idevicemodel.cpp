#include "idevicemodel.h"

#include "idevice.h"
#include "idevicewatcher.h"
#include "qurl.h"
#include <QIcon>

struct IDeviceModelPrivate {
        IDeviceWatcher* watcher;
};

IDeviceModel::IDeviceModel(IDeviceWatcher* watcher, QObject* parent) :
    QAbstractListModel(parent) {
    d = new IDeviceModelPrivate();
    d->watcher = watcher;
    connect(d->watcher, &IDeviceWatcher::addingDevice, this, [this] {
        beginResetModel();
    });
    connect(d->watcher, &IDeviceWatcher::newDevice, this, [this] {
        endResetModel();
    });
    connect(d->watcher, &IDeviceWatcher::removingDevice, this, [this] {
        beginResetModel();
    });
    connect(d->watcher, &IDeviceWatcher::removedDevice, this, [this] {
        endResetModel();
    });
}

IDeviceModel::~IDeviceModel() {
    delete d;
}

int IDeviceModel::rowCount(const QModelIndex& parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return d->watcher->devices().length();
}

QVariant IDeviceModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto device = d->watcher->devices().at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return device->deviceName();
        case Qt::DecorationRole:
            return device->icon();
        case DeviceRole:
            return QVariant::fromValue(device);
        case UrlRole:
            {
                QUrl url;
                url.setScheme("ios");
                url.setHost(QStringLiteral("u%1").arg(device->udid()));
                url.setPath("/");
                return url;
            }
    }

    return QVariant();
}
