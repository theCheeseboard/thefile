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

#include "widgets/filetransferjobwidget.h"
#include <QCoroFuture>
#include <QCoroIODevice>
#include <QElapsedTimer>
#include <QUrl>
#include <resourcemanager.h>
#include <tjobmanager.h>
#include <tlogger.h>
#include <tnotification.h>
#include <tpromise.h>

struct FileTransferJobPrivate {
        quint64 progress;
        quint64 totalProgress;
        tJob::State state = tJob::Processing;

        FileTransferJob::TransferType type;
        QList<QUrl> source;
        DirectoryPtr destination;

        QElapsedTimer timer;

        QMap<QUrl, QUrl> sourceMappings;
        QMap<QUrl, Directory::FileInformation> sourceInformation;
        QList<QUrl> conflicts;
        quint64 totalDataToTransfer;
        quint64 totalFilesTransferred = 0;

        QElapsedTimer progressUpdateTimer;

        FileTransferJob::TransferStage stage = FileTransferJob::FileDiscovery;
        QString description;

        QUrl errorSourceUrl, errorDestUrl;

        QPointer<QWidget> jobsPopover;

        bool cancelled = false;
        bool silent = false;
};

FileTransferJob::FileTransferJob(TransferType type, QList<QUrl> source, DirectoryPtr destination, QWidget* jobsPopover, QObject* parent) :
    tJob(parent) {
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
        // We can continue now!
        transferFiles();
    }
}

void FileTransferJob::resolveError(bool skip) {
    if (!skip) {
        d->sourceMappings.insert(d->errorSourceUrl, d->errorDestUrl);
    }

    d->stage = FileTransfer;
    emit transferStageChanged(FileTransfer);

    d->state = Processing;
    emit stateChanged(Processing);

    d->description = tr("Copying Files");
    emit descriptionChanged(d->description);

    transferNextFile();
}

void FileTransferJob::setSilent(bool silent) {
    d->silent = silent;
}

void FileTransferJob::cancel() {
    d->cancelled = true;
    if (d->stage == ConflictResolution || d->stage == ErrorResolution) setJobCancelled();
}

bool FileTransferJob::cancelled() {
    return d->cancelled;
}

QCoro::Task<> FileTransferJob::fileDiscovery() {
    d->stage = FileDiscovery;
    emit transferStageChanged(FileDiscovery);

    d->description = tr("Counting files to transfer...");
    emit descriptionChanged(d->description);

    struct DiscoveryResults {
            QMap<QUrl, QUrl> sourceMappings;
            QMap<QUrl, Directory::FileInformation> sourceInformation;
            quint64 totalFileSize = 0;
    };

    DiscoveryResults results;
    for (const QUrl& baseUrl : qAsConst(d->source)) {
        DirectoryPtr baseDirectory = ResourceManager::parentDirectoryForUrl(baseUrl);
        Directory::FileInformation baseFileInfo = co_await baseDirectory->fileInformation(baseUrl.fileName());

        if (baseDirectory->isFile(baseUrl.fileName())) {
            //                QUrl relativePath(baseUrl.path().remove(baseUrl.resolved(QUrl(".")).path()));
            QUrl relativePath(baseFileInfo.filenameForFileOperations);

            QUrl destUrl = d->destination->url();
            if (!destUrl.path().endsWith("/")) destUrl.setPath(destUrl.path() + "/");
            QUrl destination = destUrl.resolved(relativePath);
            results.sourceMappings.insert(baseUrl, destination);

            auto info = co_await baseDirectory->fileInformation(baseUrl.fileName());
            results.totalFileSize += info.size;
            results.sourceInformation.insert(baseUrl, info);

            d->description = tr("Counted %n files to transfer...", nullptr, results.sourceMappings.count());
            if (d->progressUpdateTimer.elapsed() > 100) {
                emit descriptionChanged(d->description);
                d->progressUpdateTimer.restart();
            }
        } else {
            QList<QUrl> sourceUrls = {baseUrl};
            while (!sourceUrls.isEmpty()) {
                QUrl sourceUrl = sourceUrls.takeFirst();
                DirectoryPtr sourceDirectory = ResourceManager::parentDirectoryForUrl(sourceUrl);
                Directory::FileInformation sourceFileInfo = co_await baseDirectory->fileInformation(sourceUrl.fileName());
                if (sourceDirectory->isFile(sourceUrl.fileName())) {
                    QString relativePathString = sourceUrl.path().remove(baseUrl.resolved(QUrl(".")).path()).chopped(sourceFileInfo.pathSegment.length()).append(sourceFileInfo.filenameForFileOperations);
                    //                        QString relativePathString = sourceFileInfo.filenameForFileOperations;
                    if (relativePathString.startsWith("/")) relativePathString.remove(0, 1);

                    QUrl destUrl = d->destination->url();
                    if (!destUrl.path().endsWith("/")) destUrl.setPath(destUrl.path() + "/");
                    QUrl destination = destUrl.resolved(QUrl(relativePathString));
                    results.sourceMappings.insert(sourceUrl, destination);

                    auto info = co_await sourceDirectory->fileInformation(sourceUrl.fileName());
                    results.totalFileSize += info.size;
                    results.sourceInformation.insert(sourceUrl, info);
                } else {
                    auto urls = co_await QtConcurrent::run([sourceUrl] {
                        QList<QUrl> sourceUrls;
                        auto directory = ResourceManager::directoryForUrl(sourceUrl);
                        auto generator = directory->list(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDir::NoSort);
                        for (const Directory::FileInformation& info : generator) {
                            sourceUrls.append(info.resource);
                        }
                        return sourceUrls;
                    });
                    sourceUrls.append(urls);
                }

                // Cancel the job if a cancellation has been requested
                if (d->cancelled) {
                    setJobCancelled();
                    co_return;
                }
            }
            d->description = tr("Counted %n files to transfer...", nullptr, results.sourceMappings.count());
            if (d->progressUpdateTimer.elapsed() > 100) {
                emit descriptionChanged(d->description);
                d->progressUpdateTimer.restart();
            }
        }
        // Cancel the job if a cancellation has been requested
        if (d->cancelled) {
            setJobCancelled();
            co_return;
        }
    }

    // Cancel the job if a cancellation has been requested
    if (d->cancelled) {
        setJobCancelled();
        co_return;
    }

    d->sourceMappings = results.sourceMappings;
    d->totalDataToTransfer = results.totalFileSize;
    d->sourceInformation = results.sourceInformation;
    conflictCheck();
}

void FileTransferJob::conflictCheck() {
    d->stage = ConflictChecking;
    emit transferStageChanged(ConflictChecking);

    d->description = tr("Checking for file conflicts");
    emit descriptionChanged(d->description);

    QFuture<QUrl> future = QtConcurrent::filtered(d->sourceMappings.keys(), [=](const QUrl& sourceUrl) {
        QUrl dest = d->sourceMappings.value(sourceUrl);
        return ResourceManager::parentDirectoryForUrl(dest)->isFile(dest.fileName());
    });

    QFutureWatcher<QUrl>* watcher = new QFutureWatcher<QUrl>();
    connect(watcher, &QFutureWatcher<QUrl>::finished, this, [=] {
        // Cancel the job if a cancellation has been requested
        if (d->cancelled) {
            setJobCancelled();
            return;
        }

        d->conflicts = watcher->future().results();

        if (d->conflicts.isEmpty()) {
            // We can start copying
            transferFiles();
        } else {
            // Enter conflict resolution mode
            d->stage = ConflictResolution;
            emit transferStageChanged(ConflictResolution);

            d->state = RequiresAttention;
            emit stateChanged(RequiresAttention);

            d->description = tr("Waiting for conflict resolution");
            emit descriptionChanged(d->description);

            if (d->timer.elapsed() < 2000 && d->jobsPopover) {
                tJobManager::showJobsPopover(d->jobsPopover);
            } else {
                if (!d->silent) {
                    tNotification* n = new tNotification();
                    n->setSummary(tr("File Conflicts"));
                    n->setText(tr("%n files in the destination folder have the same file name as files being transferred. Resolve the file conflicts to continue transferring files.", nullptr, d->conflicts.count()));
                    //                n->setText(tr("Transferring files will result in %n files coexisting with the same name in the destination. Resolve the file conflicts to continue transferring files.", nullptr, d->conflicts.count()));
                    n->insertAction("resolve", tr("Resolve File Conflicts"));
                    connect(n, &tNotification::actionClicked, this, [=](QString key) {
                        if (key == "resolve") {
                            tJobManager::showJobsPopover(d->jobsPopover);
                            d->jobsPopover->window()->activateWindow();
                        }
                    });
                    n->post();
                }
            }
        }
    });
    watcher->setFuture(future);
}

QCoro::Task<> FileTransferJob::transferFiles() {
    d->stage = FileTransfer;
    emit transferStageChanged(FileTransfer);

    d->state = Processing;
    emit stateChanged(Processing);

    d->description = tr("Copying Files");
    emit descriptionChanged(d->description);

    d->totalProgress = d->totalDataToTransfer;
    emit totalProgressChanged(d->totalProgress);

    try {
        // Start copying
        while (!d->sourceMappings.isEmpty()) {
            co_await transferNextFile();
        }

        // We're done!
        d->stage = Done;
        emit transferStageChanged(Done);

        d->state = Finished;
        emit stateChanged(Finished);

        QString notificationText;
        if (d->type == Move) {
            d->description = tr("Moved %n files", nullptr, d->totalFilesTransferred);
            notificationText = tr("Successfully moved %n files", nullptr, d->totalFilesTransferred);
        } else if (d->type == Copy) {
            d->description = tr("Copied %n files", nullptr, d->totalFilesTransferred);
            notificationText = tr("Successfully copied %n files", nullptr, d->totalFilesTransferred);
        }
        emit descriptionChanged(d->description);

        if (d->timer.elapsed() >= 2000) {
            if (!d->silent) {
                tNotification* n = new tNotification();
                n->setSummary(tr("Files Transferred"));
                n->setText(notificationText);
                n->post();
            }
        }
    } catch (DirectoryOperationException ex) {
        // Enter error resolution mode
        d->stage = ErrorResolution;
        emit transferStageChanged(ErrorResolution);

        d->state = RequiresAttention;
        emit stateChanged(RequiresAttention);

        d->description = tr("Waiting for error resolution");
        emit descriptionChanged(d->description);

        if (d->timer.elapsed() < 2000 && d->jobsPopover) {
            tJobManager::showJobsPopover(d->jobsPopover);
        } else {
            if (!d->silent) {
                tNotification* n = new tNotification();
                n->setSummary(tr("File Transfer Error"));
                n->setText(tr("An error occurred trying to transfer files."));
                n->insertAction("resolve", tr("Resolve"));
                connect(n, &tNotification::actionClicked, this, [=](QString key) {
                    if (key == "resolve") {
                        tJobManager::showJobsPopover(d->jobsPopover);
                        d->jobsPopover->window()->activateWindow();
                    }
                });
                n->post();
            }
        }
    }
}

QCoro::Task<> FileTransferJob::transferNextFile() {
    // Cancel the job if a cancellation has been requested
    if (d->cancelled) {
        setJobCancelled();
        co_return;
    }

    QUrl sourceUrl = d->sourceMappings.keys().first();
    QUrl destinationUrl = d->sourceMappings.take(sourceUrl);

    Directory::FileInformation sourceInformation = d->sourceInformation.value(sourceUrl);
    QString fileName = QFileInfo(sourceUrl.path()).fileName();

    DirectoryPtr sourceDirectory = ResourceManager::parentDirectoryForUrl(sourceUrl);
    DirectoryPtr destinationDirectory = ResourceManager::parentDirectoryForUrl(destinationUrl);

    // Determine what to do with these files
    if (d->type == Move) {
        // See if we can just move the file
        if (sourceDirectory->canMove(sourceUrl.fileName(), destinationUrl)) {
            d->description = tr("Moving %n").arg(fileName);

            // Delete the destination if it exists
            if (destinationDirectory->isFile(destinationUrl.fileName())) destinationDirectory->deleteFile(destinationUrl.fileName());
            co_await destinationDirectory->mkpath(".");
            co_await sourceDirectory->move(sourceUrl.fileName(), destinationUrl);

            d->progress += sourceInformation.size;
            d->totalFilesTransferred++;
            co_return;
        }
    }

    try {
        co_await QtConcurrent::run([this](DirectoryPtr sourceDirectory, DirectoryPtr destinationDirectory, QUrl sourceUrl, QUrl destinationUrl, Directory::FileInformation sourceInformation, QString fileName) {
            auto source = QCoro::waitFor(sourceDirectory->open(sourceUrl.fileName(), QIODevice::ReadOnly));
            QCoro::waitFor(destinationDirectory->mkpath("."));

            try {
                auto dest = QCoro::waitFor(destinationDirectory->open(destinationUrl.fileName(), QIODevice::WriteOnly | QIODevice::Unbuffered));

                // Dump the source file to the destination file
                quint64 readBytes = 0;

                while (readBytes < sourceInformation.size) {
                    QByteArray buf = source->read(1048576);
                    dest->write(buf);
                    readBytes += buf.size();

                    d->progress += buf.size();
                    d->description = tr("Copying %1\n%2 of %3\n\nTotal progress: %4 of %5").arg(fileName).arg(QLocale().formattedDataSize(readBytes)).arg(QLocale().formattedDataSize(sourceInformation.size)).arg(QLocale().formattedDataSize(d->progress)).arg(QLocale().formattedDataSize(d->totalProgress));

                    if (d->progressUpdateTimer.elapsed() > 100) {
                        emit descriptionChanged(d->description);
                        emit progressChanged(d->progress);
                        d->progressUpdateTimer.restart();
                    }

                    // Leave the file if the job has been cancelled
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
                    // Delete the source file
                    QCoro::waitFor(sourceDirectory->deleteFile(sourceUrl.fileName()));
                }
            } catch (DirectoryOperationException ex) {
                source->close();
                source->deleteLater();
                throw;
            }
        },
            sourceDirectory, destinationDirectory, sourceUrl, destinationUrl, sourceInformation, fileName);
    } catch (DirectoryOperationException ex) {
        d->errorSourceUrl = sourceUrl;
        d->errorDestUrl = destinationUrl;
        throw;
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
