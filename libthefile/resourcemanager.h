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
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QDir>
#include "schemehandler.h"

struct ResourceManagerPrivate;
class ResourceManager : public QObject {
        Q_OBJECT
    public:
        explicit ResourceManager(QObject* parent = nullptr);

        static ResourceManager* instance();

        bool registerSchemeHandler(QString scheme, SchemeHandler* handler);
        static bool isSchemeHandlerRegistered(QString scheme);

        static bool isFile(QUrl url);
        static tPromise<FileInformationList>* list(QUrl url, QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::SortFlags sortFlags = QDir::DirsFirst);
        static tPromise<SchemeHandler::FileInformation>* fileInformation(QUrl url);
        static tPromise<QIODevice*>* open(QUrl url, QIODevice::OpenMode mode);

        static tPromise<void>* mkpath(QUrl url);

        static bool canTrash(QUrl url);
        static tPromise<QUrl>* trash(QUrl url);
        static tPromise<void>* deleteFile(QUrl url);

        static bool canMove(QUrl from, QUrl to);
        static tPromise<void>* move(QUrl from, QUrl to);

        static SchemePathWatcher* watch(QUrl url);

        static QVariant special(QString scheme, QString operation, QVariantMap args);

    signals:

    private:
        ResourceManagerPrivate* d;
        static SchemeHandler* handlerForUrl(QUrl url);
};

#endif // RESOURCEMANAGER_H
