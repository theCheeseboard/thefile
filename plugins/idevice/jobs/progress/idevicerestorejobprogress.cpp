#include "idevicerestorejobprogress.h"
#include "ui_idevicerestorejobprogress.h"

#include "../idevicerestorejob.h"

struct IDeviceRestoreJobProgressPrivate {
        IDeviceRestoreJob* job;
};

IDeviceRestoreJobProgress::IDeviceRestoreJobProgress(IDeviceRestoreJob* job, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::IDeviceRestoreJobProgress) {
    ui->setupUi(this);
    d = new IDeviceRestoreJobProgressPrivate();
    d->job = job;

    ui->titleLabel->setText(job->titleString());

    connect(job, &IDeviceRestoreJob::stateChanged, this, &IDeviceRestoreJobProgress::updateState);
    connect(job, &IDeviceRestoreJob::totalProgressChanged, this, [this](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(job, &IDeviceRestoreJob::progressChanged, this, [this](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(job, &IDeviceRestoreJob::descriptionChanged, ui->statusLabel, &QLabel::setText);
    connect(job, &IDeviceRestoreJob::canCancelChanged, this, [this](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    updateState();
    ui->statusLabel->setText(job->description());
    ui->cancelButton->setEnabled(job->canCancel());
}

IDeviceRestoreJobProgress::~IDeviceRestoreJobProgress() {
    delete d;
    delete ui;
}

void IDeviceRestoreJobProgress::updateState() {
    switch (d->job->state()) {
        case tJob::Processing:
            ui->progressBar->setMaximum(d->job->totalProgress());
            ui->progressBar->setValue(d->job->progress());
            ui->progressBar->setVisible(true);
            ui->cancelButton->setVisible(true);
            break;
        case tJob::Finished:
            ui->progressBar->setMaximum(1);
            ui->progressBar->setValue(1);
            ui->progressBar->setVisible(false);
            ui->cancelButton->setVisible(false);
            break;
        case tJob::Failed:
            ui->progressBar->setMaximum(1);
            ui->progressBar->setValue(1);
            ui->progressBar->setVisible(false);
            ui->cancelButton->setVisible(false);
            break;
    }
}

void IDeviceRestoreJobProgress::on_cancelButton_clicked() {
    d->job->cancel();
}
