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
#ifndef FILETRANSFERJOB_H
#define FILETRANSFERJOB_H

#include <directory.h>
#include <tjob.h>

struct FileTransferJobPrivate;
class FileTransferJob : public tJob {
        Q_OBJECT
    public:
        enum TransferType {
            Copy,
            Move
        };

        enum TransferStage {
            FileDiscovery,
            ConflictChecking,
            ConflictResolution,
            FileTransfer,
            ErrorResolution,
            Done
        };

        explicit FileTransferJob(TransferType type, QList<QUrl> source, DirectoryPtr destination, QWidget* jobsPopover, QObject* parent = nullptr);
        ~FileTransferJob();

        TransferType type();
        TransferStage stage();

        QString description();

        QMap<QUrl, QUrl> conflictingFiles();
        void resolveConflict(QUrl sourceFile, QUrl resolveTo);
        void resolveError(bool skip);

        void setSilent(bool silent);

        void cancel();
        bool cancelled();

    signals:
        void transferStageChanged(TransferStage stage);
        void descriptionChanged(QString description);

    private:
        FileTransferJobPrivate* d;

        QCoro::Task<> fileDiscovery();
        void conflictCheck();
        QCoro::Task<> transferFiles();
        QCoro::Task<> transferNextFile();

        void setJobCancelled();

        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        State state();
        QWidget* makeProgressWidget();
        QString titleString();
        QString statusString();
};

Q_DECLARE_METATYPE(FileTransferJob::TransferStage)

#endif // FILETRANSFERJOB_H
