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

#include "libthefile_global.h"
#include <QCoroGenerator>
#include <QCoroTask>
#include <QDir>
#include <QException>
#include <QIcon>
#include <QObject>
#include <QUrl>

class Directory : public QObject,
                  public tfSharedFromThis<Directory> {
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

        virtual QCoro::Task<bool> exists() = 0;
        virtual bool isFile(QString path) = 0;
        virtual QUrl url() = 0;
        virtual quint64 listCount(QDir::Filters filters, QDir::SortFlags sortFlags) = 0;
        virtual QCoro::Generator<FileInformation> list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset = 0) = 0;
        virtual QCoro::Task<FileInformation> fileInformation(QString filename) = 0;
        virtual QCoro::Task<QIODevice*> open(QString filename, QIODevice::OpenMode mode) = 0;

        virtual QCoro::Task<> mkpath(QString filename) = 0;

        virtual bool canTrash(QString filename) = 0;
        virtual QCoro::Task<QUrl> trash(QString filename) = 0;
        virtual QCoro::Task<> deleteFile(QString filename) = 0;

        virtual bool canMove(QString filename, QUrl to) = 0;
        virtual QCoro::Task<> move(QString filename, QUrl to) = 0;

        virtual QVariant special(QString operation, QVariantMap args) = 0;

    signals:
        void contentsChanged();
};

class DirectoryOperationException : public QException {
    public:
        DirectoryOperationException(QString error) {
            this->e = error;
        };

        QString error() {
            return this->e;
        }

        void raise() const override { throw *this; }
        DirectoryOperationException* clone() const override { return new DirectoryOperationException(*this); }

    private:
        QString e;
};

typedef QList<Directory::FileInformation> FileInformationList;
typedef QSharedPointer<Directory> DirectoryPtr;

#endif // DIRECTORY_H
