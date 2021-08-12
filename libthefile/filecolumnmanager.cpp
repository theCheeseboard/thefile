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
#include "filecolumnmanager.h"

#include "filecolumn.h"

struct FileColumnManagerPrivate {
    FileColumn* current = nullptr;
    bool fileTransfersSupported = false;
    bool canOpenProperties = false;
    QList<FileTab::OpenFileButton> openFileButtons;
    QList<FileTab::ColumnAction> columnActions;
    QList<FileTab::Filter> filters;
};

FileColumnManager::FileColumnManager(QObject* parent) : QObject(parent) {
    d = new FileColumnManagerPrivate();
}

FileColumnManager::~FileColumnManager() {
    delete d;
}

void FileColumnManager::setCurrent(FileColumn* col) {
    if (d->current) d->current->disconnect(this);
    d->current = col;
    connect(col, &FileColumn::destroyed, this, [ = ] {
        if (d->current == col) d->current = nullptr;
    });

    emit currentChanged();
}

FileColumn* FileColumnManager::current() {
    return d->current;
}

void FileColumnManager::setFileTransfersSupported(bool supported) {
    d->fileTransfersSupported = supported;
}

bool FileColumnManager::fileTransfersSupported() {
    return d->fileTransfersSupported;
}

void FileColumnManager::setCanOpenProperties(bool canOpen) {
    d->canOpenProperties = canOpen;
}

bool FileColumnManager::canOpenProperties() {
    return d->canOpenProperties;
}

void FileColumnManager::setOpenFileButtons(QList<FileTab::OpenFileButton> buttons) {
    d->openFileButtons = buttons;
    emit openFileButtonsChanged(buttons);
}

QList<FileTab::OpenFileButton> FileColumnManager::openFileButtons() {
    return d->openFileButtons;
}

void FileColumnManager::setColumnActions(QList<FileTab::ColumnAction> actions) {
    d->columnActions = actions;
    emit columnActionsChanged(actions);
}

QList<FileTab::ColumnAction> FileColumnManager::columnActions() {
    return d->columnActions;
}

void FileColumnManager::setFilters(QList<FileTab::Filter> filters) {
    d->filters = filters;
    emit filtersChanged(filters);
}

QList<FileTab::Filter> FileColumnManager::filters() {
    return d->filters;
}
