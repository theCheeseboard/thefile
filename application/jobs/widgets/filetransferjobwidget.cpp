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
#include "filetransferjobwidget.h"
#include "ui_filetransferjobwidget.h"

#include <QTimer>
#include <QUrl>
#include "../filetransferjob.h"

struct FileTransferJobWidgetPrivate {
    FileTransferJob* job;
    tVariantAnimation* heightAnim = nullptr;
};

FileTransferJobWidget::FileTransferJobWidget(FileTransferJob* job, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileTransferJobWidget) {
    ui->setupUi(this);

    d = new FileTransferJobWidgetPrivate();
    d->job = job;

    auto TransferStageChangedHandler = [ = ](FileTransferJob::TransferStage stage) {
        if (stage == FileTransferJob::ConflictResolution) {
            ui->stackedWidget->setCurrentWidget(ui->conflictResolutionPage);
            ui->conflictDescriptionLabel->setText(tr("%n files in the destination folder have the same file name as files being transferred", nullptr, job->conflictingFiles().count()));
        } else if (stage == FileTransferJob::ErrorResolution) {
            ui->stackedWidget->setCurrentWidget(ui->errorResolutionPage);

            if (job->type() == FileTransferJob::Copy) {
                ui->errorDescriptionLabel->setText(tr("An error occurred while trying to copy a file."));
            } else {
                ui->errorDescriptionLabel->setText(tr("An error occurred while trying to move a file."));
            }
        } else {
            ui->stackedWidget->setCurrentWidget(ui->progressPage);
        }
    };

    ui->operationLabel->setText(job->type() == FileTransferJob::Copy ? tr("COPYING FILES") : tr("MOVING FILES"));
    ui->progressBar->setValue(job->progress() / 1000);
    ui->progressBar->setMaximum(job->totalProgress() / 1000);
    ui->statusLabel->setText(job->description());
    TransferStageChangedHandler(job->stage());

    if (job->cancelled() || job->state() != tJob::Processing) ui->cancelButton->setEnabled(false);

    connect(job, &FileTransferJob::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress / 1000);
    }, Qt::QueuedConnection);
    connect(job, &FileTransferJob::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress / 1000);
    }, Qt::QueuedConnection);
    connect(job, &FileTransferJob::descriptionChanged, this, [ = ](QString description) {
        ui->statusLabel->setText(description);
        QTimer::singleShot(0, this, [ = ] {
            performResize();
        });
    }, Qt::QueuedConnection);
    connect(job, &FileTransferJob::transferStageChanged, this, TransferStageChangedHandler, Qt::QueuedConnection);
    connect(job, &FileTransferJob::stateChanged, this, &FileTransferJobWidget::updateState, Qt::QueuedConnection);
    updateState();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    this->setFixedHeight(ui->stackedWidget->widget(ui->stackedWidget->currentIndex())->sizeHint().height());
}

FileTransferJobWidget::~FileTransferJobWidget() {
    delete d;
    delete ui;
}

void FileTransferJobWidget::on_stackedWidget_switchingFrame(int frame) {
    performResize();
}

void FileTransferJobWidget::on_replaceAllConflictsButton_clicked() {
    //Resolve each conflict with the current file name
    QMap<QUrl, QUrl> conflicting = d->job->conflictingFiles();
    for (QUrl source : conflicting.keys()) {
        d->job->resolveConflict(source, conflicting.value(source));
    }
}

void FileTransferJobWidget::on_skipAllConflictsButton_clicked() {
    //Resolve each conflict with an invalid URL
    QMap<QUrl, QUrl> conflicting = d->job->conflictingFiles();
    for (const QUrl& source : conflicting.keys()) {
        d->job->resolveConflict(source, QUrl());
    }
}

void FileTransferJobWidget::performResize() {
    QWidget* w = ui->stackedWidget->widget(ui->stackedWidget->currentIndex());
    if (this->height() == w->sizeHint().height()) return;
    if (d->heightAnim) {
        d->heightAnim->stop();
        d->heightAnim->deleteLater();
    }

    d->heightAnim = new tVariantAnimation(this);
    d->heightAnim->setStartValue(this->height());
    d->heightAnim->setEndValue(w->sizeHint().height());
    d->heightAnim->setDuration(500);
    d->heightAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->heightAnim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(d->heightAnim, &tVariantAnimation::finished, this, [ = ] {
        d->heightAnim->deleteLater();
        d->heightAnim = nullptr;
    });
    d->heightAnim->start();
}

void FileTransferJobWidget::updateState() {
    if (d->job->cancelled() || d->job->state() != tJob::Processing) {
        ui->cancelButton->setEnabled(false);
    } else {
        ui->cancelButton->setEnabled(true);
    }

    if (d->job->state() == tJob::Failed || d->job->state() == tJob::Finished) {
        ui->progressWidget->setVisible(false);
    } else {
        ui->progressWidget->setVisible(true);
    }

    performResize();
}

void FileTransferJobWidget::on_cancelButton_clicked() {
    d->job->cancel();
    ui->cancelButton->setEnabled(false);
}

void FileTransferJobWidget::on_cancelConflictsButton_clicked() {
    d->job->cancel();
}

void FileTransferJobWidget::on_skipErrorButton_clicked() {
    d->job->resolveError(true);
}

void FileTransferJobWidget::on_retryErrorButton_clicked() {
    d->job->resolveError(false);
}

void FileTransferJobWidget::on_cancelErrorButton_clicked() {
    d->job->cancel();
}
