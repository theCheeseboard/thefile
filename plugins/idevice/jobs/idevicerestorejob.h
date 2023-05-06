#ifndef IDEVICERESTOREJOB_H
#define IDEVICERESTOREJOB_H

#include <tjob.h>

class IDevice;
struct IDeviceRestoreJobPrivate;
class IDeviceRestoreJob : public tJob {
        Q_OBJECT
    public:
        explicit IDeviceRestoreJob(bool erase, IDevice* device, QObject* parent = nullptr);
        ~IDeviceRestoreJob();

        QString description();
        bool isErase();
        bool canCancel();

        QString deviceName();

        void startRestore(QString softwareFile, QString softwareVersion);
        void cancel();

    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);

    private:
        IDeviceRestoreJobPrivate* d;

        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        State state();
        QWidget* makeProgressWidget();
        QString titleString();
        QString statusString();
};

#endif // IDEVICERESTOREJOB_H
