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
#include "bookmarksmodel.h"

#include <QIcon>
#include <QUrl>
#include "bookmarkmanager.h"

BookmarksModel::BookmarksModel(QObject* parent)
    : QAbstractListModel(parent) {
    connect(BookmarkManager::instance(), &BookmarkManager::bookmarksChanged, this, [ = ] {
        emit dataChanged(index(0), index(rowCount()));
    });
}

int BookmarksModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return BookmarkManager::instance()->bookmarkCount();
}

QVariant BookmarksModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    QUrl bookmark = BookmarkManager::instance()->bookmark(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return bookmark.fileName();
        case Qt::DecorationRole:
            return QIcon::fromTheme("folder");
        case UrlRole:
            return bookmark;
    }

    return QVariant();
}
