#include "idevicewatcher.h"

#include "idevice.h"
#include <QMap>
#include <QTimer>
#include <libimobiledevice/libimobiledevice.h>
#include <tlogger.h>

struct IDeviceWatcherPrivate {
        QTimer* updateTimer;
        QMap<QString, IDevice*> devices;
};

IDeviceWatcher::IDeviceWatcher(QObject* parent) :
    QObject{parent} {
    d = new IDeviceWatcherPrivate();

    idevice_event_subscribe([](const idevice_event_t* event, void* user_data) {
        auto me = reinterpret_cast<IDeviceWatcher*>(user_data);
        switch (event->event) {
            case IDEVICE_DEVICE_ADD:
                me->addDevice(QString::fromLocal8Bit(event->udid));
                break;
            case IDEVICE_DEVICE_REMOVE:
                me->removeDevice(QString::fromLocal8Bit(event->udid));
                break;
            case IDEVICE_DEVICE_PAIRED:
                break;
        }
    },
        this);
}

IDeviceWatcher::~IDeviceWatcher() {
    delete d;
}

QList<IDevice*> IDeviceWatcher::devices() {
    return d->devices.values();
}

void IDeviceWatcher::addDevice(QString udid) {
    emit addingDevice();
    tDebug("IDeviceWatcher") << "New device with UDID " << udid;

    auto device = new IDevice(udid);
    d->devices.insert(udid, device);
    emit newDevice();
}

void IDeviceWatcher::removeDevice(QString udid) {
    emit removingDevice();
    tDebug("IDeviceWatcher") << "Removed device with UDID " << udid;

    auto device = d->devices.take(udid);
    device->deleteLater();

    emit removedDevice();
}
