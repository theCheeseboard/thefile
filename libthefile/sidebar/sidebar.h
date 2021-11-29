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
#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QListWidgetItem>
#include <QUrl>
#include <QStyledItemDelegate>
#include "directory.h"

namespace Ui {
    class Sidebar;
}

class DiskObject;
struct SidebarPrivate;
class Sidebar : public QWidget {
        Q_OBJECT

    public:
        explicit Sidebar(QWidget* parent = nullptr);
        ~Sidebar();

    private slots:
        void on_placesWidget_itemActivated(QListWidgetItem* item);

        void on_devicesView_activated(const QModelIndex& index);

        void on_devicesView_customContextMenuRequested(const QPoint& pos);

        void on_bookmarksView_activated(const QModelIndex& index);

        void on_bookmarksView_customContextMenuRequested(const QPoint& pos);

    signals:
        void navigate(QUrl location);
        void moveFiles(QList<QUrl> source, DirectoryPtr destination);
        void copyFiles(QList<QUrl> source, DirectoryPtr destination);

    private:
        Ui::Sidebar* ui;
        SidebarPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);
        void mount(DiskObject* disk);
};

class SidebarDelegate : public QStyledItemDelegate {
        Q_OBJECT
    public:
        explicit SidebarDelegate(QObject* parent = nullptr);

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // SIDEBAR_H
