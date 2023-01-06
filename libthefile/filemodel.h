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
#ifndef FILEMODEL_H
#define FILEMODEL_H

#include "directory.h"
#include "filetab.h"
#include <QAbstractListModel>
#include <QStyledItemDelegate>

struct FileModelPrivate;
class FileModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit FileModel(DirectoryPtr directory, QObject* parent = nullptr);
        ~FileModel();

        enum Roles {
            UrlRole = Qt::UserRole,
            HiddenRole,
            PathSegmentRole,
            ExcludedByFilterRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        QMimeData* mimeData(const QModelIndexList& indexes) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        void setFilters(QList<FileTab::Filter> filters);

        bool isFile();

        QString currentError();

    signals:
        void isFileChanged();

    private:
        FileModelPrivate* d;
        QCoro::Task<> reloadData();

        void incReset();
        void decReset();
};

class FileDelegate : public QStyledItemDelegate {
        Q_OBJECT
    public:
        explicit FileDelegate(QObject* parent = nullptr);

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // FILEMODEL_H
