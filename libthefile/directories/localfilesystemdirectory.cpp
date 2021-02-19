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
#include "localfilesystemdirectory.h"

#include <QDir>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QStorageInfo>
#include <unistd.h>

struct LocalFilesystemDirectoryPrivate {
    QUrl url;
    QFileIconProvider iconProvider;

    QFileSystemWatcher* watcher;
};

LocalFilesystemDirectory::LocalFilesystemDirectory(QUrl url, QObject* parent) : Directory(parent) {
    d = new LocalFilesystemDirectoryPrivate();
    d->url = url;

    d->watcher = new QFileSystemWatcher();
    d->watcher->addPath(url.toLocalFile());
    connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &LocalFilesystemDirectory::contentsChanged);
    connect(d->watcher, &QFileSystemWatcher::fileChanged, this, &LocalFilesystemDirectory::contentsChanged);
}

LocalFilesystemDirectory::~LocalFilesystemDirectory() {
    d->watcher->deleteLater();
    delete d;
}

bool LocalFilesystemDirectory::isFile(QString filename) {
    return QFileInfo(QDir(d->url.toLocalFile()).absoluteFilePath(filename)).isFile();
}

QUrl LocalFilesystemDirectory::url() {
    return d->url;
}

tPromise<FileInformationList>* LocalFilesystemDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags) {
    QUrl url = d->url;

    return TPROMISE_CREATE_NEW_THREAD(FileInformationList, {
        QDir dir(url.toLocalFile());

        QFileInfoList files = dir.entryInfoList(filters, sortFlags);

        if (files.isEmpty()) {
            QFileInfo dirInfo(d->url.toLocalFile());

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
            fileInfo.pathSegment = file.fileName();
            fileInfo.filenameForFileOperations = file.fileName();
            fileInfoList.append(fileInfo);
        }

        res(fileInfoList);
    });
}

tPromise<Directory::FileInformation>* LocalFilesystemDirectory::fileInformation(QString filename) {
    QUrl url = d->url;

    return TPROMISE_CREATE_NEW_THREAD(Directory::FileInformation, {
        Q_UNUSED(rej)

        QFileInfo file(QDir(url.toLocalFile()).absoluteFilePath(filename));
        QFileIconProvider provider;

        FileInformation fileInfo;
        fileInfo.name = file.fileName();
        fileInfo.resource = QUrl::fromLocalFile(file.filePath());
        fileInfo.size = file.size();
        fileInfo.icon = provider.icon(file);
        fileInfo.isHidden = file.isHidden();
        fileInfo.pathSegment = file.fileName();
        fileInfo.filenameForFileOperations = file.fileName();

        res(fileInfo);
    });
}

tPromise<QIODevice*>* LocalFilesystemDirectory::open(QString filename, QIODevice::OpenMode mode) {
    QUrl url = d->url;

    return TPROMISE_CREATE_SAME_THREAD(QIODevice*, {
        QFile* file = new QFile(QDir(url.toLocalFile()).absoluteFilePath(filename));
        if (file->open(mode)) {
            res(file);
        } else {
            file->deleteLater();
            rej("Can't open file");
        }
    });
}

tPromise<void>* LocalFilesystemDirectory::mkpath(QString filename) {
    QUrl url = d->url;

    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (QDir::root().mkpath(QDir(url.toLocalFile()).absoluteFilePath(filename))) {
            res();
        } else {
            rej("Could not make path");
        }
    });
}

bool LocalFilesystemDirectory::canTrash(QString filename) {
    return true;
}

tPromise<QUrl>* LocalFilesystemDirectory::trash(QString filename) {
    QUrl url = d->url;

    return TPROMISE_CREATE_NEW_THREAD(QUrl, {
        QString pathInTrash;
        if (QFile::moveToTrash(QDir(url.toLocalFile()).absoluteFilePath(filename), &pathInTrash)) {
            res(QUrl::fromLocalFile(pathInTrash));
        } else {
            rej("Could not trash file");
        }
    });
}

tPromise<void>* LocalFilesystemDirectory::deleteFile(QString filename) {
    QUrl url = d->url;

    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (isFile(filename)) {
            if (QFile::remove(QDir(url.toLocalFile()).absoluteFilePath(filename))) {
                res();
            } else {
                rej("Could not delete file");
            }
        } else {
            QDir dir(QDir(url.toLocalFile()).absoluteFilePath(filename));
            if (dir.removeRecursively()) {
                res();
            } else {
                rej("Could not delete folder");
            }
        }
    });
}

bool LocalFilesystemDirectory::canMove(QString filename, QUrl to) {
    return canMove(d->url, filename, to);
}

bool LocalFilesystemDirectory::canMove(QUrl from, QString filename, QUrl to) {
    if (to.scheme() != "file") return false;
    QDir fromDir = QFileInfo(QDir(from.toLocalFile()).absoluteFilePath(filename)).dir();
    QDir toDir = QFileInfo(to.toLocalFile()).dir();

    QStorageInfo fromVol(fromDir);
    QStorageInfo toVol(toDir);

    return fromVol.rootPath() == toVol.rootPath();
}

tPromise<bool>* LocalFilesystemDirectory::exists() {
    return TPROMISE_CREATE_SAME_THREAD(bool, {
        res(QFileInfo(d->url.toLocalFile()).isDir());
    });
}

tPromise<void>* LocalFilesystemDirectory::move(QString filename, QUrl to) {
    QUrl url = d->url;
    return TPROMISE_CREATE_NEW_THREAD(void, {
        if (!canMove(url, filename, to)) {
            rej("Cannot move");
            return;
        }

        QFile::rename(QDir(url.toLocalFile()).absoluteFilePath(filename), to.toLocalFile());

        res();
    });
}

QVariant LocalFilesystemDirectory::special(QString operation, QVariantMap args) {
    Q_UNUSED(args);
    Q_UNUSED(operation);
    return QVariant();
}
