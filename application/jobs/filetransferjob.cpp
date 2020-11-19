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
#include "filetransferjob.h"

#include <QUrl>
#include <QElapsedTimer>
#include <tjobmanager.h>
#include <tpromise.h>
#include <resourcemanager.h>
#include <tlogger.h>
#include "widgets/filetransferjobwidget.h"

struct FileTransferJobPrivate {
    quint64 progress;
    quint64 totalProgress;
    tJob::State state = tJob::Processing;

    FileTransferJob::TransferType type;
    QList<QUrl> source;
    QUrl destination;

    QElapsedTimer timer;

    QMap<QUrl, QUrl> sourceMappings;
    QMap<QUrl, SchemeHandler::FileInformation> sourceInformation;
    QList<QUrl> conflicts;
    quint64 totalDataToTransfer;
    quint64 totalFilesTransferred = 0;

    QElapsedTimer progressUpdateTimer;

    FileTransferJob::TransferStage stage = FileTransferJob::FileDiscovery;
    QString description;

    QPointer<QWidget> jobsPopover;

    bool cancelled = false;
};

FileTransferJob::FileTransferJob(TransferType type, QList<QUrl> source, QUrl destination, QWidget* jobsPopover, QObject* parent) : tJob(parent) {
    d = new FileTransferJobPrivate();
    d->type = type;
    d->source = source;
    d->destination = destination;
    d->jobsPopover = jobsPopover;
    d->timer.start();
    d->progressUpdateTimer.start();

    qRegisterMetaType<TransferStage>();

    fileDiscovery();
}

FileTransferJob::~FileTransferJob() {
    delete d;
}

FileTransferJob::TransferType FileTransferJob::type() {
    return d->type;
}

FileTransferJob::TransferStage FileTransferJob::stage() {
    return d->stage;
}

QString FileTransferJob::description() {
    return d->description;
}

QMap<QUrl, QUrl> FileTransferJob::conflictingFiles() {
    QMap<QUrl, QUrl> conflicts;
    for (QUrl url : d->conflicts) {
        conflicts.insert(url, d->sourceMappings.value(url));
    }
    return conflicts;
}

void FileTransferJob::resolveConflict(QUrl sourceFile, QUrl resolveTo) {
    if (resolveTo.isValid()) {
        d->sourceMappings.insert(sourceFile, resolveTo);
    } else {
        d->sourceMappings.remove(sourceFile);
        d->totalDataToTransfer -= d->sourceInformation.value(sourceFile).size;
    }

    d->conflicts.removeOne(sourceFile);
    if (d->conflicts.count() == 0) {
        //We can continue now!
        transferFiles();
    }
}

void FileTransferJob::cancel() {
    d->cancelled = true;
    if (d->stage == ConflictResolution) setJobCancelled();
}

bool FileTransferJob::cancelled() {
    return d->cancelled;
}

void FileTransferJob::fileDiscovery() {
    d->stage = FileDiscovery;
    emit transferStageChanged(FileDiscovery);

    d->description = tr("Counting files to transfer...");
    emit descriptionChanged(d->description);

    struct DiscoveryResults {
        QMap<QUrl, QUrl> sourceMappings;
        QMap<QUrl, SchemeHandler::FileInformation> sourceInformation;
        quint64 totalFileSize = 0;
    };

    TPROMISE_CREATE_NEW_THREAD(DiscoveryResults, {
        Q_UNUSED(rej);
        //TODO: Automatically rename files if a copy exists

        DiscoveryResults results;
        for (QUrl baseUrl : d->source) {
            if (ResourceManager::isFile(baseUrl)) {
                QUrl relativePath(baseUrl.path().remove(baseUrl.resolved(QUrl(".")).path()));
                QUrl destination = d->destination.resolved(relativePath);
                results.sourceMappings.insert(baseUrl, destination);

                tPromiseResults<SchemeHandler::FileInformation> info = ResourceManager::fileInformation(baseUrl)->await();
                results.totalFileSize += info.result.size;
                results.sourceInformation.insert(baseUrl, info.result);

                d->description = tr("Counted %n files to transfer...", nullptr, results.sourceMappings.count());
                if (d->progressUpdateTimer.elapsed() > 100) {
                    emit descriptionChanged(d->description);
                    d->progressUpdateTimer.restart();
                }
            } else {
                QList<QUrl> sourceUrls = {baseUrl};
                while (!sourceUrls.isEmpty()) {
                    QUrl sourceUrl = sourceUrls.takeFirst();
                    if (ResourceManager::isFile(sourceUrl)) {
                        QString relativePathString = sourceUrl.path().remove(baseUrl.resolved(QUrl(".")).path());
                        if (relativePathString.startsWith("/")) relativePathString.remove(0, 1);
                        QUrl destination = d->destination.resolved(QUrl(relativePathString));
                        results.sourceMappings.insert(sourceUrl, destination);

                        tPromiseResults<SchemeHandler::FileInformation> info = ResourceManager::fileInformation(sourceUrl)->await();
                        results.totalFileSize += info.result.size;
                        results.sourceInformation.insert(sourceUrl, info.result);
                    } else {
                        tPromiseResults<FileInformationList> results = ResourceManager::list(sourceUrl, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot)->await();
                        for (SchemeHandler::FileInformation info : results.result) {
                            sourceUrls.append(info.resource);
                        }
                    }

                    //Cancel the job if a cancellation has been requested
                    if (d->cancelled) {
                        res(results);
                        return;
                    }
                }
                d->description = tr("Counted %n files to transfer...", nullptr, results.sourceMappings.count());
                if (d->progressUpdateTimer.elapsed() > 100) {
                    emit descriptionChanged(d->description);
                    d->progressUpdateTimer.restart();
                }
            }
            //Cancel the job if a cancellation has been requested
            if (d->cancelled) {
                res(results);
                return;
            }
        }
        res(results);
    })->then([ = ](DiscoveryResults results) {
        //Cancel the job if a cancellation has been requested
        if (d->cancelled) {
            setJobCancelled();
            return;
        }

        d->sourceMappings = results.sourceMappings;
        d->totalDataToTransfer = results.totalFileSize;
        d->sourceInformation = results.sourceInformation;
        conflictCheck();
    });
}

void FileTransferJob::conflictCheck() {
    d->stage = ConflictChecking;
    emit transferStageChanged(ConflictChecking);

    d->description = tr("Checking for file conflicts");
    emit descriptionChanged(d->description);

    QFuture<QUrl> future = QtConcurrent::filtered(d->sourceMappings.keys(), [ = ](const QUrl & sourceUrl) {
        QUrl dest = d->sourceMappings.value(sourceUrl);
        return ResourceManager::isFile(dest);
    });

    QFutureWatcher<QUrl>* watcher = new QFutureWatcher<QUrl>();
    connect(watcher, &QFutureWatcher<QUrl>::finished, this, [ = ] {
        //Cancel the job if a cancellation has been requested
        if (d->cancelled) {
            setJobCancelled();
            return;
        }

        d->conflicts = watcher->future().results();

        if (d->conflicts.isEmpty()) {
            //We can start copying
            transferFiles();
        } else {
            //Enter conflict resolution mode
            d->stage = ConflictResolution;
            emit transferStageChanged(ConflictResolution);

            d->state = RequiresAttention;
            emit stateChanged(RequiresAttention);

            d->description = tr("Waiting for conflict resolution");
            emit descriptionChanged(d->description);

            if (d->timer.elapsed() < 2000 && d->jobsPopover) {
                tJobManager::showJobsPopover(d->jobsPopover);
            }
        }
    });
    watcher->setFuture(future);
}

void FileTransferJob::transferFiles() {
    d->stage = FileTransfer;
    emit transferStageChanged(FileTransfer);

    d->state = Processing;
    emit stateChanged(Processing);

    d->description = tr("Copying Files");
    emit descriptionChanged(d->description);

    d->totalProgress = d->totalDataToTransfer;
    emit totalProgressChanged(d->totalProgress);

    //Start copying
    //TODO: If this is a move operation AND we're on the same filesystem, do a rename

    transferNextFile();
}

void FileTransferJob::transferNextFile() {
    //Cancel the job if a cancellation has been requested
    if (d->cancelled) {
        setJobCancelled();
        return;
    }

    if (d->sourceMappings.isEmpty()) {
        //We're done!
        d->stage = Done;
        emit transferStageChanged(Done);

        d->state = Finished;
        emit stateChanged(Finished);

        d->description = tr("Copied %n files", nullptr, d->totalFilesTransferred);
        emit descriptionChanged(d->description);
    } else {
        QUrl sourceUrl = d->sourceMappings.keys().first();
        QUrl destinationUrl = d->sourceMappings.take(sourceUrl);

        TPROMISE_CREATE_NEW_THREAD(void, {
            SchemeHandler::FileInformation sourceInformation = d->sourceInformation.value(sourceUrl);
            QString fileName = QFileInfo(sourceUrl.path()).fileName();

            //Determine what to do with these files
            if (d->type == Move) {
                //See if we can just move the file
                if (ResourceManager::canMove(sourceUrl, destinationUrl)) {
                    d->description = tr("Moving %n").arg(fileName);

                    //Delete the destination if it exists
                    if (ResourceManager::isFile(destinationUrl)) ResourceManager::deleteFile(destinationUrl);
                    ResourceManager::mkpath(destinationUrl.resolved(QUrl(".")))->await();
                    tPromiseResults<void> moveResults = ResourceManager::move(sourceUrl, destinationUrl)->await();
                    if (!moveResults.error.isEmpty()) {
                        tWarn("FileTransferJob") << "Failed to move" << sourceUrl.toString() << "->" << destinationUrl.toString();
                        tWarn("FileTransferJob") << moveResults.error;
                        rej("Could not move file");
                        return;
                    }

                    d->progress += sourceInformation.size;
                    d->totalFilesTransferred++;
                    res();
                    return;
                }
            }

            tPromiseResults<QIODevice*> sourceResults = ResourceManager::open(sourceUrl, QIODevice::ReadOnly)->await();
            if (!sourceResults.error.isEmpty()) {
                //Error!
                tWarn("FileTransferJob") << "Failed to copy" << sourceUrl.toString() << "->" << destinationUrl.toString();
                tWarn("FileTransferJob") << sourceResults.error;
                rej("Could not open source file");
                return;
            }
            QIODevice* source = sourceResults.result;

            ResourceManager::mkpath(destinationUrl.resolved(QUrl(".")))->await();

            tPromiseResults<QIODevice*> destResults = ResourceManager::open(destinationUrl, QIODevice::WriteOnly | QIODevice::Unbuffered)->await();
            if (!destResults.error.isEmpty()) {
                //Error!
                source->close();
                source->deleteLater();
                tWarn("FileTransferJob") << "Failed to copy" << sourceUrl.toString() << "->" << destinationUrl.toString();
                tWarn("FileTransferJob") << destResults.error;
                rej("Could not open destination file");
                return;
            }
            QIODevice* dest = destResults.result;

            //Dump the source file to the destination file
            quint64 readBytes = 0;


            while (readBytes < sourceInformation.size) {
                QByteArray buf = source->read(1048576);
                dest->write(buf);
                readBytes += buf.count();

                d->progress += buf.count();
                d->description = tr("Copying %1\n%2 of %3\n\nTotal progress: %4 of %5").arg(fileName).arg(QLocale().formattedDataSize(readBytes)).arg(QLocale().formattedDataSize(sourceInformation.size)).arg(QLocale().formattedDataSize(d->progress)).arg(QLocale().formattedDataSize(d->totalProgress));

                if (d->progressUpdateTimer.elapsed() > 100) {
                    emit descriptionChanged(d->description);
                    emit progressChanged(d->progress);
                    d->progressUpdateTimer.restart();
                }

                //Leave the file if the job has been cancelled
                if (d->cancelled) {
                    break;
                }
            }

            d->totalFilesTransferred++;

            if (d->cancelled) {
                d->description = tr("Cancelling operation...");
                emit descriptionChanged(d->description);
            }

            source->close();
            dest->close();
            source->deleteLater();
            dest->deleteLater();


            if (d->type == Move && !d->cancelled) {
                //Delete the source file
                ResourceManager::deleteFile(sourceUrl)->await();
            }

            res();
        })->then([ = ] {
            transferNextFile();
        })->error([ = ](QString error) {
            //TODO: Alert the user and ask if they want to retry or skip
            transferNextFile();
        });
    }
}

void FileTransferJob::setJobCancelled() {
    d->stage = Done;
    emit transferStageChanged(Done);

    d->state = Failed;
    emit stateChanged(Failed);

    d->description = tr("Cancelled");
    emit descriptionChanged(d->description);

    d->progress = 1;
    emit progressChanged(d->progress);

    d->totalProgress = 1;
    emit totalProgressChanged(d->totalProgress);
}

quint64 FileTransferJob::progress() {
    return d->progress;
}

quint64 FileTransferJob::totalProgress() {
    return d->totalProgress;
}

tJob::State FileTransferJob::state() {
    return d->state;
}

QWidget* FileTransferJob::makeProgressWidget() {
    return new FileTransferJobWidget(this);
}
