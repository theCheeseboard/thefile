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
#include "bookmarkmanager.h"

#include <tsettings.h>
#include <QUrl>

struct BookmarkManagerPrivate {
    QList<QUrl> bookmarks;
    tSettings* settings;
};

BookmarkManager::BookmarkManager(QObject* parent) : QObject(parent) {
    d = new BookmarkManagerPrivate();

    tSettings::registerDefaults("theSuite", "theFile", "/etc/theSuite/theFile/defaults.conf");
    d->settings = new tSettings("theSuite", "theFile", this);

    QStringList bookmarks = d->settings->delimitedList("bookmarks/items");
    bookmarks.removeAll("");
    for (const QString& bookmark : bookmarks) {
        d->bookmarks.append(QUrl(QByteArray::fromBase64(bookmark.toUtf8())));
    }

    connect(d->settings, &tSettings::settingChanged, this, [ = ](QString key, QVariant value) {
        Q_UNUSED(value);
        if (key == "bookmarks/items") emit bookmarksChanged();
    });
}

void BookmarkManager::saveChanges() {
    QStringList bookmarks;
    for (const QUrl& bookmark : qAsConst(d->bookmarks)) {
        bookmarks.append(bookmark.toString().toUtf8().toBase64());
    }

    d->settings->setDelimitedList("bookmarks/items", bookmarks);
}

BookmarkManager* BookmarkManager::instance() {
    static BookmarkManager* instance = new BookmarkManager();
    return instance;
}

bool BookmarkManager::isBookmark(QUrl url) {
    return d->bookmarks.contains(url);
}

void BookmarkManager::addBookmark(QUrl url) {
    d->bookmarks.append(url);
    saveChanges();
}

void BookmarkManager::removeBookmark(QUrl url) {
    d->bookmarks.removeAll(url);
    saveChanges();
}

int BookmarkManager::bookmarkCount() {
    return d->bookmarks.count();
}

QUrl BookmarkManager::bookmark(int index) {
    return d->bookmarks.at(index);
}
