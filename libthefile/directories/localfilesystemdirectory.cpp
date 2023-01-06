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

#include <QCoroFuture>
#include <QDir>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QStorageInfo>
#include <QtConcurrent>
#include <unistd.h>

struct LocalFilesystemDirectoryPrivate {
        QString url;
        QFileIconProvider iconProvider;

        QFileSystemWatcher* watcher;
};

LocalFilesystemDirectory::LocalFilesystemDirectory(QUrl url, QObject* parent) :
    Directory(parent) {
    d = new LocalFilesystemDirectoryPrivate();
    d->url = url.toString();

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
    return QFileInfo(QDir(QUrl(d->url).toLocalFile()).absoluteFilePath(filename)).isFile();
}

QUrl LocalFilesystemDirectory::url() {
    return QUrl(d->url);
}

quint64 LocalFilesystemDirectory::listCount(QDir::Filters filters, QDir::SortFlags sortFlags) {
    QUrl url = d->url;
    QDir dir(url.toLocalFile());
    auto files = dir.entryList(filters, sortFlags);
    return files.length();
}

QCoro::Generator<Directory::FileInformation> LocalFilesystemDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset) {
    auto shared = this->sharedFromThis();
    QUrl url = d->url;
    QDir dir(url.toLocalFile());

    auto files = dir.entryList(filters, sortFlags);

    if (files.isEmpty()) {
        QFileInfo dirInfo(QUrl(d->url).toLocalFile());

        if (!dirInfo.exists()) {
            throw DirectoryOperationException("error.not-found");
        }
        if (!dirInfo.isDir()) {
            throw DirectoryOperationException("error.not-directory");
        }

        __gid_t groupsArray[300];
        int numGroups = getgroups(300, groupsArray);
        QList<__gid_t> groups;
        for (int i = 0; i < numGroups; i++) {
            groups.append(groupsArray[i]);
        }

        if (dirInfo.ownerId() == geteuid()) {
            if (!dirInfo.permission(QFile::ExeOwner)) {
                throw DirectoryOperationException("error.permission-denied");
            }
        } else if (groups.contains(dirInfo.groupId())) {
            if (!dirInfo.permission(QFile::ExeGroup)) {
                throw DirectoryOperationException("error.permission-denied");
            }
        } else {
            if (!dirInfo.permission(QFile::ExeOther)) {
                throw DirectoryOperationException("error.permission-denied");
            }
        }
    }

    for (auto i = offset; i < files.length(); i++) {
        co_yield fileInfo(QFileInfo(dir.absoluteFilePath(files.at(i))));
    }
}

QCoro::Task<Directory::FileInformation> LocalFilesystemDirectory::fileInformation(QString filename) {
    QUrl url = d->url;

    QFileInfo file(QDir(url.toLocalFile()).absoluteFilePath(filename));
    co_return fileInfo(file);
}

QCoro::Task<QIODevice*> LocalFilesystemDirectory::open(QString filename, QIODevice::OpenMode mode) {
    QUrl url = d->url;

    QFile* file = new QFile(QDir(url.toLocalFile()).absoluteFilePath(filename));
    if (file->open(mode)) {
        co_return file;
    } else {
        file->deleteLater();
        throw DirectoryOperationException("Can't open file");
    }
}

QCoro::Task<> LocalFilesystemDirectory::mkpath(QString filename) {
    QUrl url = d->url;

    if (!QDir::root().mkpath(QDir(url.toLocalFile()).absoluteFilePath(filename))) {
        throw DirectoryOperationException("Could not make path");
    }
    co_return;
}

bool LocalFilesystemDirectory::canTrash(QString filename) {
    return true;
}

QCoro::Task<QUrl> LocalFilesystemDirectory::trash(QString filename) {
    QUrl url = d->url;

    co_return co_await QtConcurrent::run([this](QUrl url, QString filename) {
        QString pathInTrash;
        if (QFile::moveToTrash(QDir(url.toLocalFile()).absoluteFilePath(filename), &pathInTrash)) {
            return QUrl::fromLocalFile(pathInTrash);
        } else {
            throw DirectoryOperationException("Could not trash file");
        }
    },
        url, filename);
}

QCoro::Task<> LocalFilesystemDirectory::deleteFile(QString filename) {
    QUrl url = d->url;

    co_return co_await QtConcurrent::run([this](QUrl url, QString filename) {
        if (isFile(filename)) {
            if (!QFile::remove(QDir(url.toLocalFile()).absoluteFilePath(filename))) {
                throw DirectoryOperationException("Could not delete file");
            }
        } else {
            QDir dir(QDir(url.toLocalFile()).absoluteFilePath(filename));
            if (!dir.removeRecursively()) {
                throw DirectoryOperationException("Could not delete folder");
            }
        }
    },
        url, filename);
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

Directory::FileInformation LocalFilesystemDirectory::fileInfo(QFileInfo file) {
    QFileIconProvider provider;

    FileInformation fileInfo;
    fileInfo.name = file.fileName();
    fileInfo.resource = QUrl::fromLocalFile(file.filePath());
    fileInfo.size = file.size();
    fileInfo.isHidden = file.isHidden();
    fileInfo.pathSegment = file.fileName();
    fileInfo.filenameForFileOperations = file.fileName();
    fileInfo.icon = provider.icon(file);

    QMap<QStandardPaths::StandardLocation, QString> specialIcons = {
        {QStandardPaths::DocumentsLocation, "folder-documents"},
        {QStandardPaths::DownloadLocation,  "folder-downloads"},
        {QStandardPaths::MusicLocation,     "folder-music"    },
        {QStandardPaths::PicturesLocation,  "folder-pictures" },
        {QStandardPaths::MoviesLocation,    "folder-videos"   }
    };
    for (QStandardPaths::StandardLocation location : specialIcons.keys()) {
        for (const QString& locationPath : QStandardPaths::standardLocations(location)) {
            if (file.filePath() == locationPath) {
                fileInfo.icon = QIcon::fromTheme(specialIcons.value(location));
            }
        }
    }

    return fileInfo;
}

QCoro::Task<bool> LocalFilesystemDirectory::exists() {
    co_return QFileInfo(QUrl(d->url).toLocalFile()).isDir();
}

QCoro::Task<> LocalFilesystemDirectory::move(QString filename, QUrl to) {
    QUrl url = d->url;

    co_return co_await QtConcurrent::run([this](QUrl url, QString filename, QUrl to) {
        if (!canMove(url, filename, to)) {
            throw DirectoryOperationException("Cannot move");
            return;
        }

        QFile::rename(QDir(url.toLocalFile()).absoluteFilePath(filename), to.toLocalFile());
    },
        url, filename, to);
}

QVariant LocalFilesystemDirectory::special(QString operation, QVariantMap args) {
    Q_UNUSED(args);
    Q_UNUSED(operation);
    return QVariant();
}
