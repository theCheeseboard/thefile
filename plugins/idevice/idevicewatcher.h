#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>
#include <libirecovery.h>

class AbstractIDevice;
class RecoveryIDevice;
class IDevice;
struct IDeviceWatcherPrivate;
class IDeviceWatcher : public QObject {
        Q_OBJECT
    public:
        explicit IDeviceWatcher(QObject* parent = nullptr);
        ~IDeviceWatcher();

        QList<AbstractIDevice*> allDevices();
        QList<IDevice*> devices();
        QList<RecoveryIDevice*> recoveryDevices();
        IDevice* deviceByUdid(QString udid);

    signals:
        void addingDevice();
        void newDevice();
        void removingDevice();
        void removedDevice();

        void addingRecoveryDevice();
        void newRecoveryDevice();
        void removingRecoveryDevice();
        void removedRecoveryDevice();

    private:
        IDeviceWatcherPrivate* d;

        void addDevice(QString udid);
        void removeDevice(QString udid);

        void addRecoveryDevice(quint64 ecid, irecv_mode mode);
        void removeRecoveryDevice(quint64 ecid);
};

#endif // IDEVICEWATCHER_H
