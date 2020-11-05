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
#ifndef SCHEMEHANDLER_H
#define SCHEMEHANDLER_H

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QDir>
#include <tpromise.h>

class SchemePathWatcher : public QObject {
        Q_OBJECT
    public:
        explicit SchemePathWatcher(QObject* parent = nullptr);

    signals:
        void changed();
};

class SchemeHandler : public QObject {
        Q_OBJECT
    public:
        explicit SchemeHandler(QObject* parent = nullptr);

        struct FileInformation {
            QIcon icon;
            QString name;
            QUrl resource;
            quint64 size;

            bool isHidden;
        };


        virtual bool isFile(QUrl url) = 0;
        virtual tPromise<QList<FileInformation>>* list(QUrl url, QDir::Filters filters, QDir::SortFlags sortFlags) = 0;
        virtual tPromise<FileInformation>* fileInformation(QUrl url) = 0;
        virtual tPromise<QIODevice*>* open(QUrl url, QIODevice::OpenMode mode) = 0;

        virtual tPromise<void>* mkpath(QUrl url) = 0;

        virtual bool canTrash(QUrl url) = 0;
        virtual tPromise<QUrl>* trash(QUrl url) = 0;
        virtual tPromise<void>* deleteFile(QUrl url) = 0;

        virtual bool canMove(QUrl from, QUrl to) = 0;
        virtual tPromise<void>* move(QUrl from, QUrl to) = 0;

        virtual SchemePathWatcher* watch(QUrl url) = 0;

    signals:

};

typedef QList<SchemeHandler::FileInformation> FileInformationList;

#endif // SCHEMEHANDLER_H
