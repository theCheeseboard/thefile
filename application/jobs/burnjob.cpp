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
#include "burnjob.h"

#include "filetransferjob.h"
#include "widgets/burnjobprogress.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <QProcess>
#include <resourcemanager.h>
#include <tlogger.h>
#include <tnotification.h>

struct BurnJobPrivate {
        DirectoryPtr directory;
        QString title;
        QTemporaryDir tempDir;

        quint64 progress = 0;
        quint64 totalProgress = 0;
        bool canCancel = true;
        bool cancelled = false;

        QIODevice* source = nullptr;
        quint64 dataSize;
        DiskObject* disk;

        QProcess* burnProcess;
        quint64 writtenBytes = 0;

        QString description;

        tJob::State state;
        int stage = 0;
};

BurnJob::BurnJob(QString title, DirectoryPtr directory, DiskObject* disk, QObject* parent) :
    tJob(parent) {
    d = new BurnJobPrivate();
    d->disk = disk;
    d->title = title;
    d->directory = directory;

    connect(this, &BurnJob::descriptionChanged, this, &BurnJob::statusStringChanged);

    tDebug("BurnJob") << d->tempDir.path();

    // Start a file transfer job
    FileTransferJob* transferJob = new FileTransferJob(FileTransferJob::Copy, {directory->url()}, ResourceManager::directoryForUrl(QUrl::fromLocalFile(d->tempDir.path())), nullptr);
    transferJob->setSilent(true);
    connect(transferJob, &FileTransferJob::progressChanged, this, [this](quint64 progress) {
        d->progress = progress;
        emit progressChanged(d->progress);
    });
    connect(transferJob, &FileTransferJob::totalProgressChanged, this, [this](quint64 totalProgress) {
        d->totalProgress = totalProgress * 2;
        emit progressChanged(d->totalProgress);
    });
    connect(transferJob, &FileTransferJob::stateChanged, this, [this](tJob::State state) {
        if (state == tJob::Finished) {
            // Create an ISO and burn it

            d->totalProgress = 2;
            emit totalProgressChanged(d->totalProgress);

            d->progress = 1;
            emit progressChanged(d->progress);

            prepareIso(QDir(d->tempDir.path()).absoluteFilePath(d->directory->url().fileName()));
        } else if (state == tJob::Failed || state == tJob::RequiresAttention) {
            // Error
            d->stage = -1;

            // Bail out
            d->state = Failed;
            emit stateChanged(Failed);

            d->description = tr("Failed to stage files");
            emit descriptionChanged(d->description);

            d->tempDir.remove();
        }
    });

    d->description = tr("Staging files for burn");
}

BurnJob::~BurnJob() {
    delete d;
}

void BurnJob::prepareIso(QString directory) {
    // Generate ISO file
    d->description = tr("Generating Disc Image");
    emit descriptionChanged(d->description);

    // Call mkisofs to create an ISO file
    QProcess* process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(d->tempDir.path());
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitCode, QProcess::ExitStatus status) {
        if (exitCode != 0) {
            // TODO: Error error!
            return;
        }

        QFile* file = new QFile(QDir(d->tempDir.path()).absoluteFilePath("image.iso"));
        file->open(QFile::ReadOnly);
        startRestore(file, file->size());

        process->deleteLater();
    });

    QStringList mkisofsargs = {"-o", QDir(d->tempDir.path()).absoluteFilePath("image.iso"), "-V", d->title.left(32), "-A", QApplication::applicationName(), "-r", "-J", directory};
    tDebug("cdrdao") << "Calling mkisofs with arguments" << mkisofsargs;
    process->start("mkisofs", mkisofsargs);
}

QCoro::Task<> BurnJob::startRestore(QIODevice* source, quint64 dataSize) {
    if (d->stage != 0) co_return;

    d->source = source;
    d->dataSize = dataSize;

    // Try to acquire the lock
    d->description = tr("Waiting for other jobs to finish");
    emit descriptionChanged(d->description);

    co_await d->disk->lock();
    connect(this, &BurnJob::stateChanged, this, [this] {
        // Release the lock
        if (d->state == Finished || d->state == Failed) {
            d->disk->releaseLock();
        }
    });

    tInfo("BurnJob") << "Burn operation starts";
    runNextStage();
}

bool BurnJob::canCancel() {
    return d->canCancel;
}

bool BurnJob::hasBurnStarted() {
    return d->stage >= 1;
}

void BurnJob::cancel() {
    if (!d->canCancel) return;
    d->cancelled = true;
    if (d->stage == 0) {
        d->stage = -1;

        // Bail out
        d->state = Failed;
        emit stateChanged(Failed);

        d->description = tr("Failed to burn files");
        emit descriptionChanged(d->description);

        d->tempDir.remove();

        tInfo("BurnJob") << "Burn operation cancelled";
    } else if (d->stage == 1) {
        // Terminate the burn process
        d->burnProcess->terminate();

        d->burnProcess->closeWriteChannel();
        d->writtenBytes = d->dataSize;

        d->canCancel = false;
        emit canCancelChanged(d->canCancel);
    }
}

DiskObject* BurnJob::disk() {
    return d->disk;
}

QString BurnJob::description() {
    return d->description;
}

QString BurnJob::title() {
    return d->title;
}

QCoro::Task<> BurnJob::runNextStage() {
    QString displayName = d->disk->displayName();

    d->stage++;
    switch (d->stage) {
        case 1:
            {
                // Start restoring the image
                d->description = tr("Preparing to burn");
                emit descriptionChanged(d->description);

                //            OpticalErrorTracker* errorTracker = new OpticalErrorTracker();

                d->burnProcess = new QProcess();
                connect(d->burnProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    Q_UNUSED(exitStatus);
                    if (exitCode == 0) {
                        d->source->close();
                        d->source->deleteLater();

                        runNextStage();
                    } else {
                        d->state = Failed;
                        emit stateChanged(Failed);

                        //                    if (errorTracker->detectedError()) {
                        //                        d->description = tr("Couldn't burn the disc because %1.").arg(errorTracker->errorReason());
                        //                    } else {
                        d->description = tr("Failed to burn disc");
                        //                    }
                        emit descriptionChanged(d->description);

                        tNotification* notification = new tNotification();
                        notification->setSummary(tr("Couldn't Burn %1").arg(QLocale().quoteString(d->title)));
                        notification->setText(tr("Could not burn %1 to disc.").arg(QLocale().quoteString(d->title)));
                        notification->post();

                        tCritical("BurnJob") << "Burn operation failed";
                    }
                    emit progressChanged(1);
                    emit totalProgressChanged(1);
                    d->burnProcess->deleteLater();
                    d->tempDir.remove();
                    //                errorTracker->deleteLater();
                });
                connect(d->burnProcess, &QProcess::readyRead, this, [this] {
                    QByteArray peek = d->burnProcess->peek(1024);
                    while (d->burnProcess->canReadLine() || peek.contains('\r')) {
                        QString line;
                        if (d->burnProcess->canReadLine()) {
                            line = d->burnProcess->readLine();
                        } else {
                            line = d->burnProcess->read(peek.indexOf('\r') + 1);
                        }
                        line = line.trimmed();

                        tDebug("BurnJob") << line;
                        //                    errorTracker->sendLine(line);

                        if (line.startsWith("Blanking time")) {
                            d->description = tr("Preparing to burn");
                            emit descriptionChanged(d->description);

                            d->progress = 1;
                            emit progressChanged(d->progress);

                            d->totalProgress = 2;
                            emit totalProgressChanged(d->totalProgress);
                        } else if (line.startsWith("Blanking")) {
                            d->description = tr("Erasing Disc");
                            emit descriptionChanged(d->description);
                        } else if (line.startsWith("Track")) {
                            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
                            if (parts.length() >= 5 && parts.at(3) == "of") {
                                if (parts.length() >= 12) {
                                    d->description = tr("Burning Files (%1)\n%2 of %3").arg(parts.at(11)).arg(QLocale().formattedDataSize(parts.at(2).toULongLong() * 1048576)).arg(QLocale().formattedDataSize(parts.at(4).toULongLong() * 1048576));
                                    emit descriptionChanged(d->description);
                                } else {
                                    d->description = tr("Burning Files");
                                    emit descriptionChanged(d->description);
                                }

                                d->totalProgress = parts.at(4).toInt() * 2;
                                emit totalProgressChanged(d->totalProgress);

                                d->progress = parts.at(2).toInt() + d->totalProgress / 2;
                                emit progressChanged(d->progress);
                            }
                        } else if (line.startsWith("Fixating")) {
                            d->description = tr("Finalizing Disc");
                            emit descriptionChanged(d->description);

                            d->progress = 0;
                            emit progressChanged(d->progress);

                            d->totalProgress = 0;
                            emit totalProgressChanged(d->totalProgress);

                            d->canCancel = false;
                            emit canCancelChanged(d->canCancel);
                        }

                        peek = d->burnProcess->peek(1024);
                    }
                });
                connect(d->burnProcess, &QProcess::bytesWritten, this, &BurnJob::writeBlock);

                QStringList args;
                args.append("-v");

                if (!d->disk->interface<BlockInterface>()->drive()->opticalBlank()) {
                    args.append("blank=fast");
                }

                args.append(QStringLiteral("dev=%1").arg(d->disk->interface<BlockInterface>()->blockName()));
                args.append("gracetime=0");
                args.append(QStringLiteral("tsize=%1").arg(d->dataSize));
                args.append("-");

                tInfo("BurnJob") << "Starting cdrecord with arguments" << args;
                d->burnProcess->start("cdrecord", args);

                writeBlock();

                break;
            }
        case 2:
            {
                //            d->description = tr("Ejecting Disc");
                //            emit descriptionChanged(d->description);

                //            //Eject the disc
                //            d->disk->interface<BlockInterface>()->drive()->eject()->then([ = ] {
                co_await d->disk->interface<BlockInterface>()->triggerReload();
                d->state = Finished;
                emit stateChanged(Finished);

                d->description = tr("Burn Complete");
                emit descriptionChanged(d->description);

                tInfo("BurnJob") << "Burn Operation completed successfully";

                tNotification* notification = new tNotification();
                notification->setSummary(tr("Burned %1").arg(QLocale().quoteString(d->title)));
                notification->setText(tr("The folder %1 has been burned to disc.").arg(QLocale().quoteString(d->title)));
                notification->post();
                break;
            }
    }
}

void BurnJob::writeBlock() {
    if (!d->burnProcess->isWritable()) return;

    while (d->burnProcess->bytesToWrite() < 16384) {
        QByteArray buf = d->source->read(2048);
        d->writtenBytes += buf.length();

        if (d->burnProcess->isWritable()) {
            d->burnProcess->write(buf);
        }

        if (d->writtenBytes >= d->dataSize) {
            d->burnProcess->closeWriteChannel();
            return;
        }
    }
}

quint64 BurnJob::progress() {
    return d->progress;
}

quint64 BurnJob::totalProgress() {
    return d->totalProgress;
}

tJob::State BurnJob::state() {
    return d->state;
}

QWidget* BurnJob::makeProgressWidget() {
    return new BurnJobProgress(this);
}

QString BurnJob::titleString() {
    return tr("Burn %1").arg(QLocale().quoteString(this->title()));
}

QString BurnJob::statusString() {
    return this->description();
}
