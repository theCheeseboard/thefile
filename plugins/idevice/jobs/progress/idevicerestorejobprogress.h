#ifndef IDEVICERESTOREJOBPROGRESS_H
#define IDEVICERESTOREJOBPROGRESS_H

#include <QWidget>

namespace Ui {
    class IDeviceRestoreJobProgress;
}

class IDeviceRestoreJob;
struct IDeviceRestoreJobProgressPrivate;
class IDeviceRestoreJobProgress : public QWidget {
        Q_OBJECT

    public:
        explicit IDeviceRestoreJobProgress(IDeviceRestoreJob* job, QWidget* parent = nullptr);
        ~IDeviceRestoreJobProgress();

        void updateState();

    private:
        Ui::IDeviceRestoreJobProgress* ui;
        IDeviceRestoreJobProgressPrivate* d;
};

#endif // IDEVICERESTOREJOBPROGRESS_H
