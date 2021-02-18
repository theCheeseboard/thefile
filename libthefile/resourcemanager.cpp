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
#include "resourcemanager.h"

#include <QMap>
#include "directoryHandlers/localfiledirectoryhandler.h"
#include "directoryHandlers/trashdirectoryhandler.h"

struct ResourceManagerPrivate {
    QList<DirectoryHandler*> schemeHandlers;
};

ResourceManager::ResourceManager(QObject* parent) : QObject(parent) {
    d = new ResourceManagerPrivate();
    registerDirectoryHandler(new LocalFileDirectoryHandler());
    registerDirectoryHandler(new TrashDirectoryHandler());
}

ResourceManager* ResourceManager::instance() {
    static ResourceManager* instance = new ResourceManager();
    return instance;
}

bool ResourceManager::registerDirectoryHandler(DirectoryHandler* handler) {
    if (d->schemeHandlers.contains(handler)) return false;
    d->schemeHandlers.append(handler);
    return true;
}

DirectoryPtr ResourceManager::directoryForUrl(QUrl url) {
    DirectoryPtr directory;
    for (DirectoryHandler* handler : instance()->d->schemeHandlers) {
        directory = handler->directoryForUrl(url);
        if (directory) return directory;
    }
    return directory;
}

DirectoryPtr ResourceManager::parentDirectoryForUrl(QUrl url) {
    DirectoryPtr directory;
    for (DirectoryHandler* handler : instance()->d->schemeHandlers) {
        directory = handler->parentDirectoryForUrl(url);
        if (directory) return directory;
    }
    return directory;
}

QString ResourceManager::relativePath(QUrl from, QUrl to) {
    QString rel;
    for (DirectoryHandler* handler : instance()->d->schemeHandlers) {
        rel = handler->relativePath(from, to);
        if (!rel.isEmpty()) return rel;
    }
    return rel;
}
