#include "idevicewatcher.h"

#include "idevice.h"
#include "recoveryidevice.h"
#include <QMap>
#include <QTimer>
#include <libimobiledevice/libimobiledevice.h>
#include <libirecovery.h>
#include <tlogger.h>

struct IDeviceWatcherPrivate {
        QTimer* updateTimer;
        QMap<QString, IDevice*> devices;
        QMap<quint64, RecoveryIDevice*> recoveryDevices;

        irecv_device_event_context_t irecvEventContext;
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

    irecv_init();

    auto err = irecv_device_event_subscribe(
        &d->irecvEventContext, [](const irecv_device_event_t* event, void* user_data) {
            auto me = reinterpret_cast<IDeviceWatcher*>(user_data);
            switch (event->type) {
                case IRECV_DEVICE_ADD:
                    me->addRecoveryDevice(event->device_info->ecid, event->mode);
                    break;
                case IRECV_DEVICE_REMOVE:
                    me->removeRecoveryDevice(event->device_info->ecid);
                    break;
            }
        },
        this);

    tDebug("IDeviceWatcher") << "X";
}

IDeviceWatcher::~IDeviceWatcher() {
    irecv_device_event_unsubscribe(d->irecvEventContext);
    delete d;
}

QList<AbstractIDevice*> IDeviceWatcher::allDevices() {
    QList<AbstractIDevice*> devices;
    for (auto d : d->devices.values()) {
        devices.append(d);
    }
    for (auto d : d->recoveryDevices.values()) {
        devices.append(d);
    }
    return devices;
}

QList<IDevice*> IDeviceWatcher::devices() {
    return d->devices.values();
}

QList<RecoveryIDevice*> IDeviceWatcher::recoveryDevices() {
    return d->recoveryDevices.values();
}

IDevice* IDeviceWatcher::deviceByUdid(QString udid) {
    for (auto device : d->devices.values()) {
        if (device->udid().toLower() == udid.toLower()) return device;
    }
    return nullptr;
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

void IDeviceWatcher::addRecoveryDevice(quint64 ecid, irecv_mode mode) {
    emit addingRecoveryDevice();
    tDebug("IDeviceWatcher") << "New recovery device with ECID " << ecid;

    auto device = new RecoveryIDevice(ecid);
    d->recoveryDevices.insert(ecid, device);
    emit newRecoveryDevice();
}

void IDeviceWatcher::removeRecoveryDevice(quint64 ecid) {
    emit removingRecoveryDevice();
    tDebug("IDeviceWatcher") << "Removed recovery device with ECID " << ecid;

    auto device = d->recoveryDevices.take(ecid);
    device->deleteLater();

    emit removedRecoveryDevice();
}
