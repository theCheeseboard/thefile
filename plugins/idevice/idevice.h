#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>

struct IDevicePrivate;
class IDevice : public QObject {
        Q_OBJECT
    public:
        explicit IDevice(QString udid, QObject* parent = nullptr);
        ~IDevice();

        QString deviceName();
        QString deviceClass();
        QString productType();
        quint64 ecid();

    signals:

    private:
        IDevicePrivate* d;
};

#endif // IDEVICE_H
