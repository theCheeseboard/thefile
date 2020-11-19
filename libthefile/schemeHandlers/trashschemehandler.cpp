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
#include "trashschemehandler.h"

#include "fileschemehandler.h"
#include "resourcemanager.h"
#include <QStandardPaths>
#include <QFileIconProvider>

struct TrashSchemeHandlerPrivate {
    QFileIconProvider iconProvider;
};

TrashSchemeHandler::TrashSchemeHandler(QObject* parent) : SchemeHandler(parent) {
    d = new TrashSchemeHandlerPrivate();
}

TrashSchemeHandler::~TrashSchemeHandler() {
    delete d;
}

QList<QDir> TrashSchemeHandler::trashDirs() {
    QList<QDir> trashDirs;
    QString dataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    trashDirs.append(QDir(dataHome).absoluteFilePath("Trash"));
    return trashDirs;
}

QUrl TrashSchemeHandler::trashedFile(QUrl url) {
    QUrlQuery queryString(url);
    if (queryString.hasQueryItem("trashedFile")) {
        //Open the trashed file
        return QUrl::fromLocalFile(queryString.queryItemValue("trashedFile"));
    }
    return QUrl();
}

QUrl TrashSchemeHandler::trashInfoFile(QUrl url) {
    QUrlQuery queryString(url);
    if (queryString.hasQueryItem("trashInfo")) {
        //Open the trashed file
        return QUrl::fromLocalFile(queryString.queryItemValue("trashInfo"));
    }
    return QUrl();
}

SchemeHandler::FileInformation TrashSchemeHandler::internalFileInformation(QUrl url) {
    QFileInfo trashedFile(this->trashedFile(url).toLocalFile());
    QFileInfo trashInfoFile(this->trashInfoFile(url).toLocalFile());

    QSettings infoFile(trashInfoFile.filePath(), QSettings::IniFormat);
    infoFile.beginGroup("Trash Info");

    QUrl restoreUrl = QUrl(infoFile.value("Path").toString());

    FileInformation fileInformation;
    fileInformation.name = restoreUrl.fileName();
    fileInformation.isHidden = false;
    fileInformation.icon = d->iconProvider.icon(trashedFile);
    fileInformation.resource = url;
    fileInformation.size = trashedFile.size();

    return fileInformation;
}

bool TrashSchemeHandler::isFile(QUrl url) {
    return url.hasQuery();
}

tPromise<QList<SchemeHandler::FileInformation>>* TrashSchemeHandler::list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags) {
    return TPROMISE_CREATE_NEW_THREAD(FileInformationList, {
        Q_UNUSED(rej)

        if (url.path() == "/" || url.path().isEmpty()) {
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

                    information.append(internalFileInformation(resource));
                }

            }
            res(information);
        } else {
            res(FileInformationList());
        }
    });
}

tPromise<SchemeHandler::FileInformation>* TrashSchemeHandler::fileInformation(QUrl url) {
    return TPROMISE_CREATE_SAME_THREAD(FileInformation, {
        Q_UNUSED(rej);
        res(internalFileInformation(url));
    });
}

tPromise<QIODevice*>* TrashSchemeHandler::open(QUrl url, QIODevice::OpenMode mode) {
    QUrl trashedFile = this->trashedFile(url);
    if (trashedFile.isValid()) {
        //Open the trashed file
        return ResourceManager::open(trashedFile, mode);
    }
    return TPROMISE_CREATE_SAME_THREAD(QIODevice*, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

tPromise<void>* TrashSchemeHandler::mkpath(QUrl url) {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

bool TrashSchemeHandler::canTrash(QUrl url) {
    Q_UNUSED(url);
    return false;
}

tPromise<QUrl>* TrashSchemeHandler::trash(QUrl url) {
    Q_UNUSED(url);
    return TPROMISE_CREATE_SAME_THREAD(QUrl, {
        Q_UNUSED(res);
        rej("Cannot trash");
    });
}

tPromise<void>* TrashSchemeHandler::deleteFile(QUrl url) {
    QUrl trashedFile = this->trashedFile(url);
    QUrl trashInfoFile = this->trashInfoFile(url);

    if (trashedFile.isValid() && trashInfoFile.isValid()) {
        return TPROMISE_CREATE_NEW_THREAD(void, {
            Q_UNUSED(rej)

            //Delete the trash info files
            QFile::remove(trashedFile.toLocalFile());
            QFile::remove(trashInfoFile.toLocalFile());
            res();
        });
    }
    return TPROMISE_CREATE_SAME_THREAD(void, {
        Q_UNUSED(res);
        rej("Operation not supported");
    });
}

bool TrashSchemeHandler::canMove(QUrl from, QUrl to) {
    QUrl trashedFile = this->trashedFile(from);
    if (trashedFile.isValid()) {
        return ResourceManager::canMove(trashedFile, to);
    }
    return false;
}

tPromise<void>* TrashSchemeHandler::move(QUrl from, QUrl to) {
    QUrl trashedFile = this->trashedFile(from);
    QUrl trashInfoFile = this->trashInfoFile(from);
    if (trashedFile.isValid() && trashInfoFile.isValid()) {
        return TPROMISE_CREATE_SAME_THREAD_WITH_CALLBACK_NAMES(void, outerRes, outerRej, {
            ResourceManager::move(trashedFile, to)->then([ = ] {
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

SchemePathWatcher* TrashSchemeHandler::watch(QUrl url) {
    return new FileSchemePathWatcher(QUrl::fromLocalFile(trashDirs().first().absoluteFilePath("info")));
}

QVariant TrashSchemeHandler::special(QString operation, QVariantMap args) {
    if (operation == "restorePath") {
        QUrl url = args.value("url").toUrl();
        QUrl trashInfoFile = this->trashInfoFile(url);
        if (trashInfoFile.isValid()) {
            QSettings infoFile(trashInfoFile.toLocalFile(), QSettings::IniFormat);
            infoFile.beginGroup("Trash Info");

            QString originalPath = infoFile.value("Path").toString();

            return QUrl::fromLocalFile(originalPath);
        }
    }
    return QVariant();
}
