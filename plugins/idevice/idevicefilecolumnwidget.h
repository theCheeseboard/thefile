#ifndef IDEVICEFILECOLUMNWIDGET_H
#define IDEVICEFILECOLUMNWIDGET_H

#include <filecolumnwidget.h>

namespace Ui {
    class IDeviceFileColumnWidget;
}

class IDevice;
struct IDeviceFileColumnWidgetPrivate;
class IDeviceFileColumnWidget : public FileColumnWidget {
        Q_OBJECT

    public:
        explicit IDeviceFileColumnWidget(IDevice* device, QWidget* parent = nullptr);
        ~IDeviceFileColumnWidget();

    private slots:
        void on_updateButton_clicked();

        void on_restoreButton_clicked();

    private:
        Ui::IDeviceFileColumnWidget* ui;
        IDeviceFileColumnWidgetPrivate* d;
};

#endif // IDEVICEFILECOLUMNWIDGET_H
