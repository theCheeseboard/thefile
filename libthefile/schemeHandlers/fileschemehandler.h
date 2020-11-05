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
#ifndef FILESCHEMEHANDLER_H
#define FILESCHEMEHANDLER_H

#include "schemehandler.h"

struct FileSchemePathWatcherPrivate;
class FileSchemePathWatcher : public SchemePathWatcher {
        Q_OBJECT
    public:
        explicit FileSchemePathWatcher(QUrl url, QObject* parent = nullptr);
        ~FileSchemePathWatcher();

    private:
        FileSchemePathWatcherPrivate* d;
};

struct FileSchemeHandlerPrivate;
class FileSchemeHandler : public SchemeHandler {
        Q_OBJECT
    public:
        explicit FileSchemeHandler(QObject* parent = nullptr);
        ~FileSchemeHandler();

    signals:

    private:
        FileSchemeHandlerPrivate* d;

        // SchemeHandler interface
    public:
        bool isFile(QUrl url);
        tPromise<FileInformationList>* list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags);
        tPromise<FileInformation>* fileInformation(QUrl url);
        tPromise<QIODevice*>* open(QUrl url, QIODevice::OpenMode mode);
        tPromise<void>* mkpath(QUrl url);
        bool canTrash(QUrl url);
        tPromise<QUrl>* trash(QUrl url);
        tPromise<void>* deleteFile(QUrl url);
        bool canMove(QUrl from, QUrl to);
        tPromise<void>* move(QUrl from, QUrl to);
        SchemePathWatcher* watch(QUrl url);
};

#endif // FILESCHEMEHANDLER_H
