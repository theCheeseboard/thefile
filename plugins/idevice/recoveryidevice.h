#ifndef RECOVERYIDEVICE_H
#define RECOVERYIDEVICE_H

#include "abstractidevice.h"
#include <QObject>

struct RecoveryIDevicePrivate;
class RecoveryIDevice : public AbstractIDevice {
        Q_OBJECT
    public:
        explicit RecoveryIDevice(quint64 ecid, QObject* parent = nullptr);
        ~RecoveryIDevice();

    signals:

    private:
        RecoveryIDevicePrivate* d;

        // AbstractIDevice interface
    public:
        quint64 ecid();
        QString deviceName();
        QIcon icon();
        QString productType();
        QString deviceClass();
        QString humanReadableProductVersion(QString productVersion);
};

#endif // RECOVERYIDEVICE_H
