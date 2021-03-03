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
#ifndef FILECOLUMN_H
#define FILECOLUMN_H

#include <QWidget>
#include <QUrl>
#include "directory.h"

namespace Ui {
    class FileColumn;
}

class FileColumnManager;
class QMenu;
struct FileColumnPrivate;
class FileColumn : public QWidget {
        Q_OBJECT

    public:
        explicit FileColumn(DirectoryPtr directory, FileColumnManager* manager, QWidget* parent = nullptr);
        ~FileColumn();

        void setDirectory(DirectoryPtr directory);
        void setSelected(QUrl url);

        QString columnTitle();

        void cut();
        void copy();
        void paste();
        void newFolder();
        void moveToTrash();
        void deleteFile();
        void deleteOrTrash();
        void rename();
        void selectAll();

        bool isFile();
        bool canCopyCutTrash();
        bool canPaste();

    signals:
        void navigate(DirectoryPtr directory);
        void directoryChanged();
        void canCopyCutTrashChanged(bool canCopyCutTrash);

    private slots:
        void on_folderErrorPage_customContextMenuRequested(const QPoint& pos);

        void on_folderView_doubleClicked(const QModelIndex& index);

        void on_openFileButton_clicked();

        void on_folderScroller_customContextMenuRequested(const QPoint& pos);

    private:
        Ui::FileColumn* ui;
        FileColumnPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);
        void resizeEvent(QResizeEvent* event);
        void focusInEvent(QFocusEvent* event);
        void dragEnterEvent(QDragEnterEvent* event);
        void dropEvent(QDropEvent* event);

        void reload();
        void updateItems();

        void showFloater();
        void hideFloater();
        void updateFloater();

        void addFolderMenuItems(QMenu* menu);
        void ensureUrlSelected();
};

#endif // FILECOLUMN_H
