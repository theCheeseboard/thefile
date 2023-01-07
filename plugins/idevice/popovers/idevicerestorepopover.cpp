#include "idevicerestorepopover.h"
#include "ui_idevicerestorepopover.h"

#include "isoftwareupdatefile.h"
#include "jobs/idevicerestorejob.h"
#include <QFileDialog>
#include <idevice.h>
#include <libcontemporary_global.h>
#include <tjobmanager.h>

struct IDeviceRestorePopoverPrivate {
        IDevice* device;
        bool erase;

        QString softwareVersion;
};

IDeviceRestorePopover::IDeviceRestorePopover(IDevice* device, bool erase, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::IDeviceRestorePopover) {
    ui->setupUi(this);

    d = new IDeviceRestorePopoverPrivate();
    d->device = device;
    d->erase = erase;

    ui->optionsWidget->setFixedWidth(SC_DPI(600));
    ui->restoreConfirmWidget->setFixedWidth(SC_DPI(600));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
    ui->mainStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    if (erase) {
        ui->restoreButton->setProperty("type", "destructive");
        ui->doRestoreButton->setProperty("type", "destructive");
        ui->optionsLabel->setText(tr("Restore Options"));
        ui->titleLabel->setText(tr("Restore System Software on %1").arg(QLocale().quoteString(device->deviceName())));
        ui->titleLabel_3->setText(tr("Restore System Software on %1").arg(QLocale().quoteString(device->deviceName())));
    } else {
        ui->optionsLabel->setText(tr("Update Options"));
        ui->titleLabel->setText(tr("Update System Software on %1").arg(QLocale().quoteString(device->deviceName())));
        ui->titleLabel_3->setText(tr("Update System Software on %1").arg(QLocale().quoteString(device->deviceName())));
    }

    updateRestoreState();
}

IDeviceRestorePopover::~IDeviceRestorePopover() {
    delete ui;
    delete d;
}

void IDeviceRestorePopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void IDeviceRestorePopover::updateRestoreState() {
    bool canRestore = true;
    QString restoreVersion;
    if (ui->restoreFileButton->isChecked()) {
        if (ui->restoreFileBox->text().isEmpty()) {
            canRestore = false;
            ui->updateFileInformation->setText("");
        } else {
            auto file = ISoftwareUpdateFile(ui->restoreFileBox->text());
            if (!file.isValid()) {
                canRestore = false;
                ui->updateFileInformation->setText(tr("The selected update file is invalid"));
            } else {
                if (!file.supportedProductTypes().contains(d->device->productType())) {
                    canRestore = false;
                    ui->updateFileInformation->setText(tr("The selected update file does not support this device."));
                } else {
                    canRestore = true;
                    restoreVersion = file.productVersion();

                    QStringList attributes;
                    attributes.append(tr("Software Version %1").arg(file.productVersion()));
                    attributes.append(tr("Build %1").arg(file.productBuildVersion()));
                    ui->updateFileInformation->setText(attributes.join(libContemporaryCommon::humanReadablePartJoinString()));
                }
            }
        }
    } else {
        canRestore = false;
    }

    if (canRestore) {
        QString restoreOs;
        if (d->device->deviceClass() == "iPad") {
            restoreOs = "iPadOS";
        } else {
            restoreOs = "iOS";
        }

        restoreVersion = QStringLiteral("%1 %2").arg(restoreOs, restoreVersion);
        d->softwareVersion = restoreVersion;

        ui->restoreButton->setEnabled(true);
        if (d->erase) {
            ui->restoreButton->setText(tr("Restore %1 to %2").arg(QLocale().quoteString(d->device->deviceName()), restoreVersion));
            ui->confirmText->setText(tr("Once the %1 is restored, all the data on it will be erased. You may not be able to return to the previous version.").arg(d->device->deviceClass()));
            ui->confirmText2->setText(tr("Once the restore is complete, %1 will be installed onto %2.").arg(restoreVersion, QLocale().quoteString(d->device->deviceName())));
            ui->confirmText3->setText(tr("Apple will be contacted to verify the restore."));
            ui->doRestoreButton->setText(tr("Restore %1 to %2").arg(QLocale().quoteString(d->device->deviceName()), restoreVersion));
        } else {
            ui->restoreButton->setText(tr("Update %1 to %2").arg(QLocale().quoteString(d->device->deviceName()), restoreVersion));
            ui->confirmText->setText(tr("Once the %1 is updated, you may not be able to return to the previous version.").arg(d->device->deviceClass()));
            ui->confirmText2->setText(tr("Once the update is complete, %1 will be installed onto %2.").arg(restoreVersion, QLocale().quoteString(d->device->deviceName())));
            ui->confirmText3->setText(tr("Apple will be contacted to verify the update."));
            ui->doRestoreButton->setText(tr("Update %1 to %2").arg(QLocale().quoteString(d->device->deviceName()), restoreVersion));
        }
    } else {
        ui->restoreButton->setEnabled(false);
        if (d->erase) {
            ui->restoreButton->setText(tr("Restore %1").arg(QLocale().quoteString(d->device->deviceName())));
        } else {
            ui->restoreButton->setText(tr("Update %1").arg(QLocale().quoteString(d->device->deviceName())));
        }
    }
}

void IDeviceRestorePopover::on_latestVersionButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->latestVersionPage);
        updateRestoreState();
    }
}

void IDeviceRestorePopover::on_restoreFileButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->updateFilePage);
        updateRestoreState();
    }
}

void IDeviceRestorePopover::on_browseButton_clicked() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setNameFilters({"Apple System Software (*.ipsw)"});
    dialog->setFileMode(QFileDialog::AnyFile);
    connect(dialog, &QFileDialog::fileSelected, this, [this](QString file) {
        ui->restoreFileBox->setText(file);
    });
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->open();
}

void IDeviceRestorePopover::on_restoreFileBox_textChanged(const QString& arg1) {
    updateRestoreState();
}

void IDeviceRestorePopover::on_titleLabel_3_backButtonClicked() {
    ui->mainStack->setCurrentWidget(ui->mainPage);
}

void IDeviceRestorePopover::on_restoreButton_clicked() {
    ui->mainStack->setCurrentWidget(ui->confirmPage);
}

void IDeviceRestorePopover::on_doRestoreButton_clicked() {
    auto restoreJob = new IDeviceRestoreJob(d->erase, d->device);
    if (ui->restoreFileButton->isChecked()) {
        restoreJob->startRestore(ui->restoreFileBox->text(), d->softwareVersion);
    }
    tJobManager::trackJob(restoreJob);
    emit done();
}
