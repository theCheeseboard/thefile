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
#ifndef TRASHSCHEMEHANDLER_H
#define TRASHSCHEMEHANDLER_H

#include "schemehandler.h"

struct TrashSchemeHandlerPrivate;
class TrashSchemeHandler : public SchemeHandler {
        Q_OBJECT
    public:
        explicit TrashSchemeHandler(QObject* parent = nullptr);
        ~TrashSchemeHandler();

    signals:

    private:
        TrashSchemeHandlerPrivate* d;

        QList<QDir> trashDirs();

        QUrl trashedFile(QUrl url);
        QUrl trashInfoFile(QUrl url);

        FileInformation internalFileInformation(QUrl url);

        // SchemeHandler interface
    public:
        bool isFile(QUrl url);
        tPromise<QList<FileInformation>>* list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags);
        tPromise<FileInformation>* fileInformation(QUrl url);
        tPromise<QIODevice*>* open(QUrl url, QIODevice::OpenMode mode);
        tPromise<void>* mkpath(QUrl url);
        bool canTrash(QUrl url);
        tPromise<QUrl>* trash(QUrl url);
        tPromise<void>* deleteFile(QUrl url);
        bool canMove(QUrl from, QUrl to);
        tPromise<void>* move(QUrl from, QUrl to);
        SchemePathWatcher* watch(QUrl url);
        QVariant special(QString operation, QVariantMap args);
};
Q_DECLARE_METATYPE(tPromise<void>*)

#endif // TRASHSCHEMEHANDLER_H
