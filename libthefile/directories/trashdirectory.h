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
#ifndef TRASHDIRECTORY_H
#define TRASHDIRECTORY_H

#include "directory.h"

struct TrashDirectoryPrivate;
class TrashDirectory : public Directory {
        Q_OBJECT
    public:
        explicit TrashDirectory(QUrl url, QObject* parent = nullptr);
        ~TrashDirectory();

    signals:

    private:
        TrashDirectoryPrivate* d;

        QList<QDir> trashDirs();

        QUrl trashedFile(QString filename);
        QUrl trashInfoFile(QString filename);

        FileInformation internalFileInformation(QString filename);

        // SchemeHandler interface
    public:
        QCoro::Task<bool> exists();
        bool isFile(QString filename);
        QUrl url();
        quint64 listCount(QDir::Filters filters, QDir::SortFlags sortFlags);
        QCoro::Generator<FileInformation> list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset = 0);
        QCoro::Task<FileInformation> fileInformation(QString filename);
        QCoro::Task<QIODevice*> open(QString filename, QIODevice::OpenMode mode);
        QCoro::Task<> mkpath(QString filename);
        bool canTrash(QString filename);
        QCoro::Task<QUrl> trash(QString filename);
        QCoro::Task<> deleteFile(QString filename);
        bool canMove(QString filename, QUrl to);
        QCoro::Task<> move(QString filename, QUrl to);
        QVariant special(QString operation, QVariantMap args);
        QList<FileColumnWidget*> actions();
        ViewType viewType();
        QString columnTitle();
};

#endif // TRASHDIRECTORY_H
