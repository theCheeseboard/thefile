#ifndef IDEVICERESTOREJOB_H
#define IDEVICERESTOREJOB_H

#include <QCoroTask>
#include <tjob.h>

class AbstractIDevice;
struct IDeviceRestoreJobPrivate;
class IDeviceRestoreJob : public tJob {
        Q_OBJECT
    public:
        explicit IDeviceRestoreJob(bool erase, AbstractIDevice* device, QObject* parent = nullptr);
        ~IDeviceRestoreJob();

        QString description();
        bool isErase();
        bool canCancel();

        QString deviceName();

        QCoro::Task<> downloadAndStartRestore(QString buildId, QString softwareVersion, QString sha256);
        void startRestore(QString softwareFile, QString softwareVersion);
        void cancel();

    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);

    private:
        IDeviceRestoreJobPrivate* d;

        QCoro::Task<QString> downloadSoftware(QString buildId, QString softwareVersion);
        QCoro::Task<bool> checkSoftwareFile(QString softwareFile, QString sha256);

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
