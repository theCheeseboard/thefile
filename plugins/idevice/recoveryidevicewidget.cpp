#include "recoveryidevicewidget.h"
#include "ui_recoveryidevicewidget.h"

#include "popovers/idevicerestorepopover.h"
#include "recoveryidevice.h"
#include <QPointer>
#include <tpopover.h>

struct RecoveryIDeviceWidgetPrivate {
        QPointer<RecoveryIDevice> device;
};

RecoveryIDeviceWidget::RecoveryIDeviceWidget(RecoveryIDevice* device, QWidget* parent) :
    FileColumnWidget(parent),
    ui(new Ui::RecoveryIDeviceWidget) {
    ui->setupUi(this);

    d = new RecoveryIDeviceWidgetPrivate();
    d->device = device;

    ui->restoreButton->setProperty("type", "destructive");
}

RecoveryIDeviceWidget::~RecoveryIDeviceWidget() {
    delete d;
    delete ui;
}

void RecoveryIDeviceWidget::on_updateButton_clicked() {
    if (!d->device) return;

    IDeviceRestorePopover* jp = new IDeviceRestorePopover(d->device, false);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(-200);
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
    popover->show(this->window());
}

void RecoveryIDeviceWidget::on_restoreButton_clicked() {
    if (!d->device) return;

    IDeviceRestorePopover* jp = new IDeviceRestorePopover(d->device, true);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(-200);
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
    popover->show(this->window());
}
