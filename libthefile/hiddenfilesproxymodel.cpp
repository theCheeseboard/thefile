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
#include "hiddenfilesproxymodel.h"

#include "filemodel.h"
#include <tsettings.h>

struct HiddenFilesProxyModelPrivate {
        tSettings settings{"theCheeseboard", "theFile"};
};

HiddenFilesProxyModel::HiddenFilesProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent) {
    d = new HiddenFilesProxyModelPrivate();
    connect(&d->settings, &tSettings::settingChanged, this, [=](QString key, QVariant value) {
        if (key == "View/HiddenFiles") invalidateFilter();
    });
}

HiddenFilesProxyModel::~HiddenFilesProxyModel() {
    delete d;
}

bool HiddenFilesProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    if (d->settings.value("View/HiddenFiles").toBool()) return true;

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    return !index.data(FileModel::HiddenRole).toBool();
}
