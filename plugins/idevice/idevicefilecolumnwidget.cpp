#include "idevicefilecolumnwidget.h"
#include "popovers/idevicerestorepopover.h"
#include "tpopover.h"
#include "ui_idevicefilecolumnwidget.h"

#include "idevice.h"
#include <libcontemporary_global.h>

struct IDeviceFileColumnWidgetPrivate {
        IDevice* device;
};

IDeviceFileColumnWidget::IDeviceFileColumnWidget(IDevice* device, QWidget* parent) :
    FileColumnWidget(parent),
    ui(new Ui::IDeviceFileColumnWidget) {
    ui->setupUi(this);
    d = new IDeviceFileColumnWidgetPrivate();
    d->device = device;

    if (!device) {
        this->setVisible(false);
        return;
    }

    ui->deviceName->setText(device->deviceName());

    QStringList stats;
    stats.append(device->deviceClass());
    ui->deviceStats->setText(stats.join(libContemporaryCommon::humanReadablePartJoinString()));

    auto deviceIcon = device->icon().pixmap(SC_DPI_WT(QSize(32, 32), QSize, this));
    ui->deviceIcon->setPixmap(libContemporaryCommon::getTintedPixmap(deviceIcon, this->palette().color(QPalette::WindowText)));

    ui->softwareInformationLabel->setText(tr("This %1 is currently running %2").arg(d->device->deviceClass(), d->device->humanReadableProductVersion()));
}

IDeviceFileColumnWidget::~IDeviceFileColumnWidget() {
    delete d;
    delete ui;
}

void IDeviceFileColumnWidget::on_updateButton_clicked() {
    if (!d->device) return;

    IDeviceRestorePopover* jp = new IDeviceRestorePopover(d->device, false);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
    popover->show(this->window());
}

void IDeviceFileColumnWidget::on_restoreButton_clicked() {
    if (!d->device) return;

    IDeviceRestorePopover* jp = new IDeviceRestorePopover(d->device, true);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
    popover->show(this->window());
}
