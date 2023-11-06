#ifndef RECOVERYIDEVICEWIDGET_H
#define RECOVERYIDEVICEWIDGET_H

#include <filecolumnwidget.h>

namespace Ui {
    class RecoveryIDeviceWidget;
}

class RecoveryIDevice;
struct RecoveryIDeviceWidgetPrivate;
class RecoveryIDeviceWidget : public FileColumnWidget {
        Q_OBJECT

    public:
        explicit RecoveryIDeviceWidget(RecoveryIDevice* device, QWidget* parent = nullptr);
        ~RecoveryIDeviceWidget();

    private slots:
        void on_updateButton_clicked();

        void on_restoreButton_clicked();

    private:
        Ui::RecoveryIDeviceWidget* ui;
        RecoveryIDeviceWidgetPrivate* d;
};

#endif // RECOVERYIDEVICEWIDGET_H
