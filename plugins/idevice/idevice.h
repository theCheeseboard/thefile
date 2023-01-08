#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>

struct IDevicePrivate;
class IDevice : public QObject {
        Q_OBJECT
    public:
        explicit IDevice(QString udid, QObject* parent = nullptr);
        ~IDevice();

        QString udid();
        QString deviceName();
        QString deviceClass();
        QString productType();
        QString productVersion();
        quint64 ecid();

        QIcon icon();

        QString humanReadableProductVersion();
        QString humanReadableProductVersion(QString productVersion);

    signals:

    private:
        IDevicePrivate* d;
};

#endif // IDEVICE_H
