#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

class IDevice;
struct IDeviceWatcherPrivate;
class IDeviceWatcher : public QObject {
        Q_OBJECT
    public:
        explicit IDeviceWatcher(QObject* parent = nullptr);
        ~IDeviceWatcher();

        QList<IDevice*> devices();
        IDevice* deviceByUdid(QString udid);

    signals:
        void addingDevice();
        void newDevice();
        void removingDevice();
        void removedDevice();

    private:
        IDeviceWatcherPrivate* d;

        void addDevice(QString udid);
        void removeDevice(QString udid);
};

#endif // IDEVICEWATCHER_H
