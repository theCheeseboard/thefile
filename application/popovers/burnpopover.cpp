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
#include "burnpopover.h"
#include "ui_burnpopover.h"

#include <DriveObjects/diskobject.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/driveinterface.h>
#include <driveobjectmanager.h>
#include "jobs/burnjob.h"
#include <tjobmanager.h>

struct BurnPopoverPrivate {
    DirectoryPtr dir;
    DiskObject* currentBurner = nullptr;
};

BurnPopover::BurnPopover(DirectoryPtr dir, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BurnPopover) {
    ui->setupUi(this);
    d = new BurnPopoverPrivate();
    d->dir = dir;

    ui->optionsWidget->setFixedWidth(SC_DPI(600));
    ui->burnConfirmWidget->setFixedWidth(SC_DPI(600));

    ui->titleLabel->setBackButtonShown(true);
    ui->titleLabel_2->setBackButtonShown(true);

    ui->discNameEdit->setText(dir->url().fileName());

    for (DiskObject* disk : DriveObjectManager::opticalDisks()) {
        ui->burnerBox->addItem(disk->displayName(), disk->path().path());
    }

    QPalette pal = ui->warningFrame->palette();
    pal.setColor(QPalette::Window, QColor(255, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->warningFrame->setPalette(pal);
}

BurnPopover::~BurnPopover() {
    delete d;
    delete ui;
}

void BurnPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void BurnPopover::on_discNameEdit_textChanged(const QString& arg1) {
    ui->titleLabel->setText(tr("Burn %1").arg(QLocale().quoteString(arg1)));
    ui->titleLabel_2->setText(tr("Burn %1").arg(QLocale().quoteString(arg1)));
}

void BurnPopover::updateBurner() {
    DriveInterface* drive = d->currentBurner->interface<BlockInterface>()->drive();
    if (!QList<DriveInterface::MediaFormat>({
    DriveInterface::CdR,
    DriveInterface::CdRw,
    DriveInterface::DvdR,
    DriveInterface::DvdRw,
    DriveInterface::DvdPR,
    DriveInterface::DvdPRw,
    DriveInterface::DvdPRDl,
    DriveInterface::DvdPRwDl,
    DriveInterface::BdR,
    DriveInterface::BdRe
}).contains(drive->media())) {
        ui->warningText->setText(tr("Insert a burnable disc into the drive."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
    } else if (!drive->opticalBlank() && drive->media() == DriveInterface::CdR) {
        ui->warningText->setText(tr("The disc in the drive has already been written."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
//    } else if (capacity < d->playlistLength && capacity != 0) {
//        ui->warningText->setText(tr("This playlist is too long to fit on the CD."));
//        ui->warningFrame->setVisible(true);
//        ui->burnButton->setEnabled(false);
    } else {
        ui->warningFrame->setVisible(false);
        ui->burnButton->setEnabled(true);
    }
}

void BurnPopover::performBurn() {
    BurnJob* job = new BurnJob(ui->discNameEdit->text(), d->dir, d->currentBurner);
    tJobManager::trackJob(job);
    emit done();
}

void BurnPopover::on_burnerBox_currentIndexChanged(int index) {
    if (d->currentBurner) {
        d->currentBurner->interface<BlockInterface>()->drive()->disconnect(this);
        d->currentBurner->disconnect(this);
    }
    d->currentBurner = DriveObjectManager::diskForPath(QDBusObjectPath(ui->burnerBox->itemData(index).toString()));

    connect(d->currentBurner->interface<BlockInterface>()->drive(), &DriveInterface::changed, this, &BurnPopover::updateBurner);
    updateBurner();
}

void BurnPopover::on_burnButton_clicked() {
    if (!d->currentBurner->interface<BlockInterface>()->drive()->opticalBlank()) {
        ui->mainStack->setCurrentWidget(ui->confirmOpticalPage);
    } else {
        performBurn();
    }
}

void BurnPopover::on_doBurnButton_clicked() {
    performBurn();
}

void BurnPopover::on_titleLabel_2_backButtonClicked() {
    ui->mainStack->setCurrentWidget(ui->mainPage);
}
