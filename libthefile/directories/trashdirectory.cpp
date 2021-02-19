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
#include "trashdirectory.h"

#include "localfilesystemdirectory.h"
#include "resourcemanager.h"
#include <QStandardPaths>
#include <QFileIconProvider>
#include <QFileInfo>

struct TrashDirectoryPrivate {
    QFileIconProvider iconProvider;
    QUrl url;

    QFileSystemWatcher* watcher;
};

TrashDirectory::TrashDirectory(QUrl url, QObject* parent) : Directory(parent) {
    d = new TrashDirectoryPrivate();
    d->url = url;

    d->watcher = new QFileSystemWatcher();
    d->watcher->addPath(trashDirs().first().absoluteFilePath("info"));
    connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &LocalFilesystemDirectory::contentsChanged);
    connect(d->watcher, &QFileSystemWatcher::fileChanged, this, &LocalFilesystemDirectory::contentsChanged);
}

TrashDirectory::~TrashDirectory() {
    d->watcher->deleteLater();
    delete d;
}

QList<QDir> TrashDirectory::trashDirs() {
    QList<QDir> trashDirs;
    QString dataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    trashDirs.append(QDir(dataHome).absoluteFilePath("Trash"));
    return trashDirs;
}

QUrl TrashDirectory::trashedFile(QString filename) {
    QUrlQuery queryString(QByteArray::fromBase64(filename.toUtf8(), QByteArray::Base64UrlEncoding));
    if (queryString.hasQueryItem("trashedFile")) {
        //Open the trashed file
        return QUrl::fromLocalFile(queryString.queryItemValue("trashedFile"));
    }
    return QUrl();
}

QUrl TrashDirectory::trashInfoFile(QString filename) {
    QUrlQuery queryString(QByteArray::fromBase64(filename.toUtf8(), QByteArray::Base64UrlEncoding));
    if (queryString.hasQueryItem("trashInfo")) {
        //Open the trashed file
        return QUrl::fromLocalFile(queryString.queryItemValue("trashInfo"));
    }
    return QUrl();
}

Directory::FileInformation TrashDirectory::internalFileInformation(QString filename) {
    QFileInfo trashedFile(this->trashedFile(filename).toLocalFile());
    QFileInfo trashInfoFile(this->trashInfoFile(filename).toLocalFile());

    QSettings infoFile(trashInfoFile.filePath(), QSettings::IniFormat);
    infoFile.beginGroup("Trash Info");

    QUrl restoreUrl = QUrl(infoFile.value("Path").toString());

    FileInformation fileInformation;
    fileInformation.name = restoreUrl.fileName();
    fileInformation.isHidden = false;
    fileInformation.icon = d->iconProvider.icon(trashedFile);
    fileInformation.resource = QUrl("trash:///" + filename);
    fileInformation.size = trashedFile.size();
    fileInformation.pathSegment = filename;
    fileInformation.filenameForFileOperations = restoreUrl.fileName();

    return fileInformation;
}

tPromise<bool>* TrashDirectory::exists() {
    return TPROMISE_CREATE_SAME_THREAD(bool, {
        res(d->url.path() == "/");
    });
}

bool TrashDirectory::isFile(QString filename) {
//    return filename.hasQuery();
    return true;
}

QUrl TrashDirectory::url() {
    return d->url;
}

tPromise<QList<Directory::FileInformation>>* TrashDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags) {
    return TPROMISE_CREATE_NEW_THREAD(FileInformationList, {
        Q_UNUSED(rej)

        if (d->url.path() == "/" || d->url.path().isEmpty()) {
            FileInformationList information;
            for (QDir trashDir : trashDirs()) {
                QDir infoDir = trashDir.absoluteFilePath("info");
                QDir fileDir = trashDir.absoluteFilePath("files");

                QFileInfoList fileList = infoDir.entryInfoList(filters, sortFlags);
                for (QFileInfo file : fileList) {
                    QFileInfo trashedFile(fileDir.absoluteFilePath(file.fileName().remove(".trashinfo")));

                    QUrl resource;
                    resource.setScheme("trash");
                    resource.setPath(trashedFile.fileName());

                    QUrlQuery queryString;
                    queryString.setQueryItems({{"trashInfo", file.filePath()}, {"trashedFile", trashedFile.filePath()}});
                    resource.setQuery(queryString);

                    information.append(internalFileInformation(queryString.toString().toUtf8().toBase64(QByteArray::Base64UrlEncoding)));
                }

            }
            res(information);
        } else {
            res(FileInformationList());
        }
    });
}

tPromise<Directory::FileInformation>* TrashDirectory::fileInformation(QString filename) {
    return TPROMISE_CREATE_SAME_THREAD(FileInformation, {
        Q_UNUSED(rej);
        res(internalFileInformation(filename));
    });
}

tPromise<QIODevice*>* TrashDirectory::open(QString filename, QIODevice::OpenMode mode) {
    QUrl trashedFile = this->trashedFile(filename);
    if (trashedFile.isValid()) {
        //Open the trashed file
        return ResourceManager::parentDirectoryForUrl(trashedFile)->open(trashedFile.fileName(), mode);
    }
    return TPROMISE_CREATE_SAME_THREAD(QIODevice*, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

tPromise<void>* TrashDirectory::mkpath(QString filename) {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

bool TrashDirectory::canTrash(QString filename) {
    Q_UNUSED(filename);
    return false;
}

tPromise<QUrl>* TrashDirectory::trash(QString filename) {
    Q_UNUSED(filename);
    return TPROMISE_CREATE_SAME_THREAD(QUrl, {
        Q_UNUSED(res);
        rej("Cannot trash");
    });
}

tPromise<void>* TrashDirectory::deleteFile(QString filename) {
    QUrl trashedFile = this->trashedFile(filename);
    QUrl trashInfoFile = this->trashInfoFile(filename);

    if (trashedFile.isValid() && trashInfoFile.isValid()) {
        return TPROMISE_CREATE_NEW_THREAD(void, {
            Q_UNUSED(rej)

            //Delete the trash info files
            if (QFileInfo(trashedFile.toLocalFile()).isDir()) {
                QDir(trashedFile.toLocalFile()).removeRecursively();
            } else {
                QFile::remove(trashedFile.toLocalFile());
            }
            QFile::remove(trashInfoFile.toLocalFile());
            res();
        });
    }
    return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

bool TrashDirectory::canMove(QString filename, QUrl to) {
    QUrl trashedFile = this->trashedFile(filename);
    if (trashedFile.isValid()) {
        return ResourceManager::parentDirectoryForUrl(trashedFile)->canMove(trashedFile.fileName(), to);
    }
    return false;
}

tPromise<void>* TrashDirectory::move(QString filename, QUrl to) {
    QUrl trashedFile = this->trashedFile(filename);
    QUrl trashInfoFile = this->trashInfoFile(filename);
    if (trashedFile.isValid() && trashInfoFile.isValid()) {
        return TPROMISE_CREATE_SAME_THREAD_WITH_CALLBACK_NAMES(void, outerRes, outerRej, {
            ResourceManager::parentDirectoryForUrl(trashedFile)->move(trashedFile.fileName(), to)->then([ = ] {
                QFile::remove(trashInfoFile.toLocalFile());
                outerRes();
            })->error(outerRej);
        });
    }
    return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res)
        rej("Operation not supported");
    });
}

QVariant TrashDirectory::special(QString operation, QVariantMap args) {
    if (operation == "restorePath") {
        QUrl url = args.value("url").toUrl();
        QUrl trashInfoFile = this->trashInfoFile(url.fileName());
        if (trashInfoFile.isValid()) {
            QSettings infoFile(trashInfoFile.toLocalFile(), QSettings::IniFormat);
            infoFile.beginGroup("Trash Info");

            QString originalPath = infoFile.value("Path").toString();

            return QUrl::fromLocalFile(originalPath);
        }
    }
    return QVariant();
}
