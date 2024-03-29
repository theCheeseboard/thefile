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

#include <QDir>
#include <QMimeData>
#include <QMimeDatabase>
#include <QTimer>
#include <QUrl>
#include <resourcemanager.h>
#include <tlogger.h>

struct FileModelPrivate {
        DirectoryPtr currentDir;
        QList<Directory::FileInformation> files;
        quint64 count = 0;
        bool isFile = false;
        QList<FileTab::Filter> filters;

        QString currentError;
        int resetCount = 0;
};

FileModel::FileModel(DirectoryPtr directory, QObject* parent) :
    QAbstractListModel(parent) {
    d = new FileModelPrivate();
    d->currentDir = directory;

    connect(directory.data(), &Directory::contentsChanged, this, &FileModel::reloadData);

    reloadData();
}

FileModel::~FileModel() {
    delete d;
}

int FileModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->count;
}

QVariant FileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    if (d->files.length() <= index.row()) {
        auto generator = d->currentDir->list(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::DirsFirst | QDir::IgnoreCase, d->files.length());
        auto iterator = generator.begin();
        while (d->files.length() <= index.row()) {
            if (iterator == generator.end()) return QVariant();
            d->files.append(*iterator);
            ++iterator;
        }
    }

    Directory::FileInformation file = d->files.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return file.name;
        case Qt::DecorationRole:
            return file.icon;
        case UrlRole:
            return file.resource;
        case HiddenRole:
            return file.isHidden;
        case PathSegmentRole:
            return file.pathSegment;
        case ExcludedByFilterRole:
            {
                if (d->filters.isEmpty()) return false;
                if (!d->currentDir->isFile(file.name)) return false;

                QMimeDatabase db;
                for (FileTab::Filter filter : d->filters) {
                    if (filter.isMimeFilter) {
                        if (db.mimeTypeForUrl(file.resource).name() == filter.filter) return false;
                    } else {
                        QRegularExpression regex(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(filter.filter)));
                        if (regex.match(file.name).hasMatch()) return false;
                    }
                }
                return true;
            }
    }

    return QVariant();
}

QString FileModel::currentError() {
    return d->currentError;
}

QCoro::Task<> FileModel::reloadData() {
    incReset();

    try {
        d->count = d->currentDir->listCount(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::DirsFirst | QDir::IgnoreCase);
        d->files.clear();

        if (d->count == 0) {
            d->currentError = "error.no-items";
        } else {
            d->currentError = "";
        }
    } catch (DirectoryOperationException ex) {
        tWarn("FileModel") << "Could not list files:" << ex.error();
        d->files.clear();
        d->currentError = ex.error();
    }

    decReset();

    d->isFile = !co_await d->currentDir->exists();
    emit isFileChanged();
}

void FileModel::incReset() {
    if (d->resetCount == 0) beginResetModel();
    d->resetCount++;
}

void FileModel::decReset() {
    d->resetCount--;
    if (d->resetCount == 0) endResetModel();
}

FileDelegate::FileDelegate(QObject* parent) :
    QStyledItemDelegate(parent) {
}

void FileDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionViewItem newOptions = option;

    if (index.data(FileModel::HiddenRole).toBool() || index.data(FileModel::ExcludedByFilterRole).toBool()) {
        newOptions.palette.setColor(QPalette::WindowText, newOptions.palette.color(QPalette::Disabled, QPalette::WindowText));
    }

    QStyledItemDelegate::paint(painter, newOptions, index);
}

QMimeData* FileModel::mimeData(const QModelIndexList& indexes) const {
    QList<QUrl> urls;
    for (QModelIndex index : indexes) {
        urls.append(index.data(UrlRole).toUrl());
    }

    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(urls);
    return mimeData;
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    if (index.isValid()) {
        flags |= Qt::ItemIsDragEnabled;
    }
    if (index.data(FileModel::ExcludedByFilterRole).toBool()) {
        flags &= ~Qt::ItemIsSelectable;
    }
    return flags;
}

void FileModel::setFilters(QList<FileTab::Filter> filters) {
    d->filters = filters;
    emit dataChanged(index(0), index(rowCount()));
}

bool FileModel::isFile() {
    return d->isFile;
}
