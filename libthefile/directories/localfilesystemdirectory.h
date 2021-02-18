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

struct FileSchemePathWatcherPrivate;
class FileSchemePathWatcher : public SchemePathWatcher {
        Q_OBJECT
    public:
        explicit FileSchemePathWatcher(QUrl url, QObject* parent = nullptr);
        ~FileSchemePathWatcher();

    private:
        FileSchemePathWatcherPrivate* d;
};

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

        // Directory interface
    public:
        bool isFile(QString filename);
        QUrl url();
        tPromise<FileInformationList>* list(QDir::Filters filters, QDir::SortFlags sortFlags);
        tPromise<FileInformation>* fileInformation(QString filename);
        tPromise<QIODevice*>* open(QString filename, QIODevice::OpenMode mode);
        tPromise<void>* mkpath(QString filename);
        bool canTrash(QString filename);
        tPromise<QUrl>* trash(QString filename);
        tPromise<void>* deleteFile(QString filename);
        bool canMove(QString filename, QUrl to);
        tPromise<void>* move(QString filename, QUrl to);
        QVariant special(QString operation, QVariantMap args);
};

#endif // LOCALFILESYSTEMDIRECTORY_H
