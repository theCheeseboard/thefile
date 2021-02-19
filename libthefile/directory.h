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
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QDir>
#include <tpromise.h>
class Directory : public QObject {
        Q_OBJECT
    public:
        explicit Directory(QObject* parent = nullptr);

        struct FileInformation {
            QIcon icon;
            QString name;
            QUrl resource;
            quint64 size;
            QString pathSegment;
            QString filenameForFileOperations;

            bool isHidden;
        };


        virtual tPromise<bool>* exists() = 0;
        virtual bool isFile(QString path) = 0;
        virtual QUrl url() = 0;
        virtual tPromise<QList<FileInformation>>* list(QDir::Filters filters, QDir::SortFlags sortFlags) = 0;
        virtual tPromise<FileInformation>* fileInformation(QString filename) = 0;
        virtual tPromise<QIODevice*>* open(QString filename, QIODevice::OpenMode mode) = 0;

        virtual tPromise<void>* mkpath(QString filename) = 0;

        virtual bool canTrash(QString filename) = 0;
        virtual tPromise<QUrl>* trash(QString filename) = 0;
        virtual tPromise<void>* deleteFile(QString filename) = 0;

        virtual bool canMove(QString filename, QUrl to) = 0;
        virtual tPromise<void>* move(QString filename, QUrl to) = 0;

        virtual QVariant special(QString operation, QVariantMap args) = 0;

    signals:
        void contentsChanged();
};

typedef QList<Directory::FileInformation> FileInformationList;
typedef QSharedPointer<Directory> DirectoryPtr;

#endif // DIRECTORY_H
