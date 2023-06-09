#include "idevice.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <plist/plist++.h>
#include <tlogger.h>

struct IDevicePrivate {
        idevice_t device;
        lockdownd_client_t lockdown;

        QString udid;
        QString deviceName;
        QString deviceClass;
        QString productType;
        QString productVersion;
        quint64 ecid;
};

IDevice::IDevice(QString udid, QObject* parent) :
    QObject{parent} {
    d = new IDevicePrivate();
    d->udid = udid;

    idevice_new(&d->device, udid.toLocal8Bit().data());
    lockdownd_client_new(d->device, &d->lockdown, "thefile");

    plist_t lockdowndResponse;
    auto resp = lockdownd_get_value(d->lockdown, nullptr, nullptr, &lockdowndResponse);
    if (resp != LOCKDOWN_E_SUCCESS) {
        d->deviceName = tr("Unknown Device");
        d->ecid = 0;
        return;
    }

    auto plist = PList::Node::FromPlist(lockdowndResponse);
    auto node = PList::Dictionary(lockdowndResponse);
    for (auto item = node.Begin(); item != node.End(); item++) {
        auto key = QString::fromStdString(item->first);
        QVariant value;

        switch (item->second->GetType()) {
            case PLIST_BOOLEAN:
            case PLIST_UINT:
                {
                    auto uint = static_cast<PList::Integer*>(item->second);
                    value = static_cast<quint64>(uint->GetValue());
                    break;
                }
            case PLIST_REAL:
                {
                    auto real = static_cast<PList::Real*>(item->second);
                    value = real->GetValue();
                    break;
                }
            case PLIST_STRING:
                {
                    auto string = static_cast<PList::String*>(item->second);
                    value = QString::fromStdString(string->GetValue());
                    break;
                }
            case PLIST_ARRAY:
            case PLIST_DICT:
            case PLIST_DATE:
            case PLIST_DATA:
            case PLIST_KEY:
            case PLIST_UID:
            case PLIST_NULL:
            case PLIST_NONE:
                break;
        }

        if (key == "DeviceClass") {
            d->deviceClass = value.toString();
        } else if (key == "DeviceName") {
            d->deviceName = value.toString();
        } else if (key == "ProductType") {
            d->productType = value.toString();
        } else if (key == "ProductVersion") {
            d->productVersion = value.toString();
        } else if (key == "UniqueChipID") {
            d->ecid = value.toULongLong();
        }
    }
}

IDevice::~IDevice() {
    lockdownd_client_free(d->lockdown);
    idevice_free(d->device);
    delete d;
}

QString IDevice::udid() {
    return d->udid;
}

QString IDevice::deviceName() {
    return d->deviceName;
}

QString IDevice::deviceClass() {
    return d->deviceClass;
}

QString IDevice::productType() {
    return d->productType;
}

QString IDevice::productVersion() {
    return d->productVersion;
}

quint64 IDevice::ecid() {
    return d->ecid;
}

QIcon IDevice::icon() {
    if (d->deviceClass == "iPhone") {
        return QIcon::fromTheme("phone");
    } else if (d->deviceClass == "iPod") {
        return QIcon::fromTheme("phone");
    } else if (d->deviceClass == "iPad") {
        return QIcon::fromTheme("tablet");
    } else if (d->deviceClass == "Apple TV") {
        return QIcon::fromTheme("video-display");
    } else {
        return QIcon::fromTheme("phone");
    }
}

QString IDevice::humanReadableProductVersion() {
    return humanReadableProductVersion(d->productVersion);
}

QString IDevice::humanReadableProductVersion(QString productVersion) {
    if (d->deviceClass == "iPad") {
        return QStringLiteral("iPadOS %1").arg(productVersion);
    } else {
        return QStringLiteral("iOS %1").arg(productVersion);
    }
}
