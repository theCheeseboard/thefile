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
#include "filemodel.h"

#include <QUrl>
#include <QDir>
#include <tlogger.h>
#include <resourcemanager.h>

struct FileModelPrivate {
    QUrl currentPath;
    QList<SchemeHandler::FileInformation> files;
    SchemePathWatcher* watcher;

    QString currentError;
};

FileModel::FileModel(QUrl path, QObject* parent)
    : QAbstractListModel(parent) {
    d = new FileModelPrivate();
    d->currentPath = path;
    d->watcher = ResourceManager::watch(path);
    connect(d->watcher, &SchemePathWatcher::changed, this, &FileModel::reloadData);

    reloadData();
}

FileModel::~FileModel() {
    delete d;
}

int FileModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->files.count();
}

QVariant FileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    SchemeHandler::FileInformation file = d->files.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return file.name;
        case Qt::DecorationRole:
            return file.icon;
        case UrlRole:
            return file.resource;
        case HiddenRole:
            return file.isHidden;
    }

    return QVariant();
}

QString FileModel::currentError() {
    return d->currentError;
}

void FileModel::reloadData() {
    beginResetModel();

    ResourceManager::list(d->currentPath, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::DirsFirst)->then([ = ] (FileInformationList fileInfo) {
        d->files = fileInfo;
        if (d->files.isEmpty()) {
            d->currentError = "error.no-items";
        } else {
            d->currentError = "";
        }
        endResetModel();
    })->error([ = ](QString error) {
        tWarn("FileModel") << "Could not list files:" << error;
        d->files.clear();
        d->currentError = error;
        endResetModel();
    });
}


FileDelegate::FileDelegate(QObject* parent) : QStyledItemDelegate(parent) {

}

void FileDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionViewItem newOptions = option;

    if (index.data(FileModel::HiddenRole).toBool()) {
        newOptions.palette.setColor(QPalette::WindowText, newOptions.palette.color(QPalette::Disabled, QPalette::WindowText));
    }

    QStyledItemDelegate::paint(painter, newOptions, index);
}
