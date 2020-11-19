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
#include "fileschemehandler.h"

#include <QDir>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QStorageInfo>
#include <unistd.h>

struct FileSchemePathWatcherPrivate {
    QFileSystemWatcher* watcher;
};

struct FileSchemeHandlerPrivate {
    QFileIconProvider iconProvider;
};

FileSchemeHandler::FileSchemeHandler(QObject* parent) : SchemeHandler(parent) {
    d = new FileSchemeHandlerPrivate();
}

FileSchemeHandler::~FileSchemeHandler() {
    delete d;
}

bool FileSchemeHandler::isFile(QUrl url) {
    return QFileInfo(url.toLocalFile()).isFile();
}

tPromise<FileInformationList>* FileSchemeHandler::list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags) {
    return TPROMISE_CREATE_NEW_THREAD(FileInformationList, {
        QDir dir(url.toLocalFile());

        QFileInfoList files = dir.entryInfoList(filters, sortFlags);

        if (files.isEmpty()) {
            QFileInfo dirInfo(url.toLocalFile());

            if (!dirInfo.exists()) {
                rej("error.not-found");
                return;
            }
            if (!dirInfo.isDir()) {
                rej("error.not-directory");
                return;
            }

            __gid_t groupsArray[300];
            int numGroups = getgroups(300, groupsArray);
            QList<__gid_t> groups;
            for (int i = 0; i < numGroups; i++) {
                groups.append(groupsArray[i]);
            }

            if (dirInfo.ownerId() == geteuid()) {
                if (!dirInfo.permission(QFile::ExeOwner)) {
                    rej("error.permission-denied");
                    return;
                }
            } else if (groups.contains(dirInfo.groupId())) {
                if (!dirInfo.permission(QFile::ExeGroup)) {
                    rej("error.permission-denied");
                    return;
                }
            } else {
                if (!dirInfo.permission(QFile::ExeOther)) {
                    rej("error.permission-denied");
                    return;
                }
            }
        }

        FileInformationList fileInfoList;

        for (QFileInfo file : files) {
            FileInformation fileInfo;
            fileInfo.name = file.fileName();
            fileInfo.resource = QUrl::fromLocalFile(file.filePath());
            fileInfo.size = file.size();
            fileInfo.icon = d->iconProvider.icon(file);
            fileInfo.isHidden = file.isHidden();
            fileInfoList.append(fileInfo);
        }

        res(fileInfoList);
    });
}

tPromise<SchemeHandler::FileInformation>* FileSchemeHandler::fileInformation(QUrl url) {
    return TPROMISE_CREATE_NEW_THREAD(SchemeHandler::FileInformation, {
        Q_UNUSED(rej)

        QFileInfo file(url.toLocalFile());

        FileInformation fileInfo;
        fileInfo.name = file.fileName();
        fileInfo.resource = QUrl::fromLocalFile(file.filePath());
        fileInfo.size = file.size();
        fileInfo.icon = d->iconProvider.icon(file);
        fileInfo.isHidden = file.isHidden();

        res(fileInfo);
    });
}

tPromise<QIODevice*>* FileSchemeHandler::open(QUrl url, QIODevice::OpenMode mode) {
    return TPROMISE_CREATE_SAME_THREAD(QIODevice*, {
        QFile* file = new QFile(url.toLocalFile());
        if (file->open(mode)) {
            res(file);
        } else {
            file->deleteLater();
            rej("Can't open file");
        }
    });
}

tPromise<void>* FileSchemeHandler::mkpath(QUrl url) {
    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (QDir::root().mkpath(url.toLocalFile())) {
            res();
        } else {
            rej("Could not make path");
        }
    });
}

bool FileSchemeHandler::canTrash(QUrl url) {
    return true;
}

tPromise<QUrl>* FileSchemeHandler::trash(QUrl url) {
    return TPROMISE_CREATE_NEW_THREAD(QUrl, {
        QString pathInTrash;
        if (QFile::moveToTrash(url.toLocalFile(), &pathInTrash)) {
            res(QUrl::fromLocalFile(pathInTrash));
        } else {
            rej("Could not trash file");
        }
    });
}

tPromise<void>* FileSchemeHandler::deleteFile(QUrl url) {
    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (isFile(url)) {
            if (QFile::remove(url.toLocalFile())) {
                res();
            } else {
                rej("Could not delete file");
            }
        } else {
            QDir dir(url.toLocalFile());
            if (dir.removeRecursively()) {
                res();
            } else {
                rej("Could not delete folder");
            }
        }
    });
}

bool FileSchemeHandler::canMove(QUrl from, QUrl to) {
    if (from.scheme() != to.scheme()) return false;
    QDir fromDir = QFileInfo(from.toLocalFile()).dir();
    QDir toDir = QFileInfo(to.toLocalFile()).dir();

    QStorageInfo fromVol(fromDir);
    QStorageInfo toVol(toDir);

    return fromVol.rootPath() == toVol.rootPath();
}

tPromise<void>* FileSchemeHandler::move(QUrl from, QUrl to) {
    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (!canMove(from, to)) {
            rej("Cannot move");
            return;
        }

        QFile::rename(from.toLocalFile(), to.toLocalFile());

        res();
    });
}

SchemePathWatcher* FileSchemeHandler::watch(QUrl url) {
    return new FileSchemePathWatcher(url);
}

QVariant FileSchemeHandler::special(QString operation, QVariantMap args) {
    Q_UNUSED(args);
    Q_UNUSED(operation);
    return QVariant();
}

FileSchemePathWatcher::FileSchemePathWatcher(QUrl url, QObject* parent) : SchemePathWatcher(parent) {
    d = new FileSchemePathWatcherPrivate();
    d->watcher = new QFileSystemWatcher();
    d->watcher->addPath(url.toLocalFile());
    connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &FileSchemePathWatcher::changed);
    connect(d->watcher, &QFileSystemWatcher::fileChanged, this, &FileSchemePathWatcher::changed);
}

FileSchemePathWatcher::~FileSchemePathWatcher() {
    d->watcher->deleteLater();
    delete d;
}
