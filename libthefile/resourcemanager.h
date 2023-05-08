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

#include "directory.h"
#include "directoryhandler.h"
#include <QDir>
#include <QObject>

struct ResourceManagerPrivate;
class ResourceManager : public QObject {
        Q_OBJECT
    public:
        explicit ResourceManager(QObject* parent = nullptr);

        static ResourceManager* instance();

        bool registerDirectoryHandler(DirectoryHandler* handler);

        static DirectoryPtr directoryForUrl(QUrl url);
        static DirectoryPtr parentDirectoryForUrl(QUrl url);
        static QString relativePath(QUrl from, QUrl to);

    signals:

    private:
        ResourceManagerPrivate* d;
};

#endif // RESOURCEMANAGER_H
