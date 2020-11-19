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
#include "schemeHandlers/fileschemehandler.h"
#include "schemeHandlers/trashschemehandler.h"

struct ResourceManagerPrivate {
    QMap<QString, SchemeHandler*> schemeHandlers;
};

ResourceManager::ResourceManager(QObject* parent) : QObject(parent) {
    d = new ResourceManagerPrivate();
    registerSchemeHandler("file", new FileSchemeHandler());
    registerSchemeHandler("trash", new TrashSchemeHandler());
}

ResourceManager* ResourceManager::instance() {
    static ResourceManager* instance = new ResourceManager();
    return instance;
}

bool ResourceManager::registerSchemeHandler(QString scheme, SchemeHandler* handler) {
    if (d->schemeHandlers.contains(scheme)) return false;
    d->schemeHandlers.insert(scheme, handler);
    return true;
}

bool ResourceManager::isSchemeHandlerRegistered(QString scheme) {
    return instance()->d->schemeHandlers.contains(scheme);
}

bool ResourceManager::isFile(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return false;
    return handler->isFile(url);
}

tPromise<FileInformationList>* ResourceManager::list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(FileInformationList, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->list(url, filters, sortFlags);
}

tPromise<SchemeHandler::FileInformation>* ResourceManager::fileInformation(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(SchemeHandler::FileInformation, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->fileInformation(url);
}

tPromise<QIODevice*>* ResourceManager::open(QUrl url, QIODevice::OpenMode mode) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(QIODevice*, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->open(url, mode);
}

tPromise<void>* ResourceManager::mkpath(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->mkpath(url);
}

bool ResourceManager::canTrash(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return false;
    return handler->canTrash(url);
}

tPromise<QUrl>* ResourceManager::trash(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(QUrl, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->trash(url);
}

tPromise<void>* ResourceManager::deleteFile(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->deleteFile(url);
}

bool ResourceManager::canMove(QUrl from, QUrl to) {
    SchemeHandler* handler = handlerForUrl(from);
    if (!handler) return false;
    return handler->canMove(from, to);
}

tPromise<void>* ResourceManager::move(QUrl from, QUrl to) {
    SchemeHandler* handler = handlerForUrl(from);
    if (!handler) return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res)
        rej("Scheme unregistered");
    });
    return handler->move(from, to);
}

SchemePathWatcher* ResourceManager::watch(QUrl url) {
    SchemeHandler* handler = handlerForUrl(url);
    if (!handler) return nullptr;
    return handler->watch(url);
}

QVariant ResourceManager::special(QString scheme, QString operation, QVariantMap args) {
    SchemeHandler* handler = instance()->d->schemeHandlers.value(scheme);
    if (!handler) return QVariant();
    return handler->special(operation, args);
}

SchemeHandler* ResourceManager::handlerForUrl(QUrl url) {
    return instance()->d->schemeHandlers.value(url.scheme());
}
