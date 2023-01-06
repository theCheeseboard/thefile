#ifndef IDEVICEWATCHER_H
#define IDEVICEWATCHER_H

#include <QObject>

struct IDeviceWatcherPrivate;
class IDeviceWatcher : public QObject {
        Q_OBJECT
    public:
        explicit IDeviceWatcher(QObject* parent = nullptr);
        ~IDeviceWatcher();

    signals:
        void newDevice();
        void removedDevice();

    private:
        IDeviceWatcherPrivate* d;

        void addDevice(QString udid);
        void removeDevice(QString udid);
};

#endif // IDEVICEWATCHER_H
