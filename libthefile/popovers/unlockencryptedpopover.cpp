/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "unlockencryptedpopover.h"
#include "ui_unlockencryptedpopover.h"

#include <terrorflash.h>
#include <driveobjectmanager.h>
#include <DriveObjects/encryptedinterface.h>

struct UnlockEncryptedPopoverPrivate {
    DiskObject* disk;
};

UnlockEncryptedPopover::UnlockEncryptedPopover(DiskObject* disk, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UnlockEncryptedPopover) {
    ui->setupUi(this);

    d = new UnlockEncryptedPopoverPrivate();
    d->disk = disk;
    connect(DriveObjectManager::instance(), &DriveObjectManager::diskRemoved, this, [ = ](DiskObject * disk) {
        if (disk == d->disk) emit reject();
    });
    connect(d->disk, &DiskObject::interfaceRemoved, this, [ = ](DiskInterface::Interfaces interface) {
        if (interface == DiskInterface::Encrypted) emit reject();
    });

    ui->deviceNameLabel->setText(d->disk->displayName());
    ui->titleLabel->setBackButtonShown(true);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
}

UnlockEncryptedPopover::~UnlockEncryptedPopover() {
    delete ui;
    delete d;
}

void UnlockEncryptedPopover::on_titleLabel_backButtonClicked() {
    emit reject();
}

void UnlockEncryptedPopover::on_okPasswordButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
    d->disk->interface<EncryptedInterface>()->unlock(ui->passwordBox->text())->then([ = ](DiskObject * object) {
        emit accept(object);
    })->error([ = ](QString error) {
        ui->stackedWidget->setCurrentWidget(ui->passwordPage);
        tErrorFlash::flashError(ui->passwordBoxContainer);
    });
}
