#include "recoveryidevice.h"

#include <QIcon>
#include <tlogger.h>

#include <libirecovery.h>

struct RecoveryIDevicePrivate {
        quint64 ecid;
        QString productType;
};

RecoveryIDevice::RecoveryIDevice(quint64 ecid, QObject* parent) :
    AbstractIDevice{parent} {
    d = new RecoveryIDevicePrivate();
    d->ecid = ecid;

    irecv_client_t client;
    irecv_open_with_ecid(&client, ecid);

    irecv_device_t device;
    irecv_devices_get_device_by_client(client, &device);
    d->productType = QString::fromLocal8Bit(device->product_type);

    irecv_close(client);
}

RecoveryIDevice::~RecoveryIDevice() {
    delete d;
}

quint64 RecoveryIDevice::ecid() {
    return d->ecid;
}

QString RecoveryIDevice::deviceName() {
    return tr("Device in Recovery Mode");
}

QIcon RecoveryIDevice::icon() {
    return QIcon::fromTheme("phone");
}

QString RecoveryIDevice::productType() {
    return d->productType;
}

QString RecoveryIDevice::deviceClass() {
    return tr("Device");
}

QString RecoveryIDevice::humanReadableProductVersion(QString productVersion) {
    return tr("Software version %1").arg(productVersion);
}
