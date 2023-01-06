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
#ifndef LOCALFILESYSTEMDIRECTORY_H
#define LOCALFILESYSTEMDIRECTORY_H

#include "directory.h"

struct LocalFilesystemDirectoryPrivate;
class LocalFilesystemDirectory : public Directory {
        Q_OBJECT
    public:
        explicit LocalFilesystemDirectory(QUrl url, QObject* parent = nullptr);
        ~LocalFilesystemDirectory();

    signals:
        void changed();

    private:
        LocalFilesystemDirectoryPrivate* d;

        static bool canMove(QUrl from, QString filename, QUrl to);
        FileInformation fileInfo(QFileInfo file);

        // Directory interface
    public:
        QCoro::Task<bool> exists();
        bool isFile(QString filename);
        QUrl url();
        quint64 listCount(QDir::Filters filters, QDir::SortFlags sortFlags);
        QCoro::Generator<FileInformation> list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset = 0);
        QCoro::Task<FileInformation> fileInformation(QString filename);
        QCoro::Task<QIODevice*> open(QString filename, QIODevice::OpenMode mode);
        QCoro::Task<> mkpath(QString filename);
        bool canTrash(QString filename);
        QCoro::Task<QUrl> trash(QString filename);
        QCoro::Task<> deleteFile(QString filename);
        bool canMove(QString filename, QUrl to);
        QCoro::Task<> move(QString filename, QUrl to);
        QVariant special(QString operation, QVariantMap args);
};

#endif // LOCALFILESYSTEMDIRECTORY_H
