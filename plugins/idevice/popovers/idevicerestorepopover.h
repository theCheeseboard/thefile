#ifndef IDEVICERESTOREPOPOVER_H
#define IDEVICERESTOREPOPOVER_H

#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class IDeviceRestorePopover;
}

class IDevice;
struct IDeviceRestorePopoverPrivate;
class IDeviceRestorePopover : public QWidget {
        Q_OBJECT

    public:
        explicit IDeviceRestorePopover(IDevice* device, bool erase, QWidget* parent = nullptr);
        ~IDeviceRestorePopover();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_latestVersionButton_toggled(bool checked);

        void on_restoreFileButton_toggled(bool checked);

        void on_browseButton_clicked();

        void on_restoreFileBox_textChanged(const QString& arg1);

        void on_titleLabel_3_backButtonClicked();

        void on_restoreButton_clicked();

        void on_doRestoreButton_clicked();

    private:
        Ui::IDeviceRestorePopover* ui;
        IDeviceRestorePopoverPrivate* d;

        void updateRestoreState();
        QCoro::Task<> getLatestVersion();
};

#endif // IDEVICERESTOREPOPOVER_H
