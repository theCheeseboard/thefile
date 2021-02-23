/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#include "burnjobprogress.h"
#include "ui_burnjobprogress.h"

#include <DriveObjects/diskobject.h>
#include "../burnjob.h"

struct BurnJobProgressPrivate {
    BurnJob* job;
};

BurnJobProgress::BurnJobProgress(BurnJob* job, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BurnJobProgress) {
    ui->setupUi(this);

    d = new BurnJobProgressPrivate();
    d->job = job;

    ui->titleLabel->setText(tr("Burn to %1").arg(job->disk()->displayName()).toUpper());

    connect(job, &BurnJob::stateChanged, this, [ = ](BurnJob::State state) {
        updateState();
    });
    connect(job, &BurnJob::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(job, &BurnJob::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(job, &BurnJob::descriptionChanged, ui->statusLabel, &QLabel::setText);
    updateState();
    ui->statusLabel->setText(job->description());
}

BurnJobProgress::~BurnJobProgress() {
    delete d;
    delete ui;
}

void BurnJobProgress::updateState() {
    switch (d->job->state()) {
        case tJob::Processing:
            ui->progressBar->setMaximum(d->job->totalProgress());
            ui->progressBar->setValue(d->job->progress());
            break;
        case tJob::Finished:
            ui->progressBar->setMaximum(1);
            ui->progressBar->setValue(1);
            break;
        case tJob::Failed:
            ui->progressBar->setMaximum(1);
            ui->progressBar->setValue(1);
            break;
    }
}
