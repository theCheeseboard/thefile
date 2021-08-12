/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef FILECOLUMNMANAGER_H
#define FILECOLUMNMANAGER_H

#include <QObject>
#include "filetab.h"

class FileColumn;
struct FileColumnManagerPrivate;
class FileColumnManager : public QObject {
        Q_OBJECT
    public:
        explicit FileColumnManager(QObject* parent = nullptr);
        ~FileColumnManager();

        void setCurrent(FileColumn* col);
        FileColumn* current();

        void setFileTransfersSupported(bool supported);
        bool fileTransfersSupported();

        void setCanOpenProperties(bool canOpen);
        bool canOpenProperties();

        void setOpenFileButtons(QList<FileTab::OpenFileButton> buttons);
        QList<FileTab::OpenFileButton> openFileButtons();

        void setColumnActions(QList<FileTab::ColumnAction> actions);
        QList<FileTab::ColumnAction> columnActions();

        void setFilters(QList<FileTab::Filter> filters);
        QList<FileTab::Filter> filters();

    signals:
        void currentChanged();
        void openFileButtonsChanged(QList<FileTab::OpenFileButton> buttons);
        void columnActionsChanged(QList<FileTab::ColumnAction> actions);
        void filtersChanged(QList<FileTab::Filter> filters);

    private:
        FileColumnManagerPrivate* d;
};

#endif // FILECOLUMNMANAGER_H
