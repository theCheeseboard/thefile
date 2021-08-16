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
#ifndef FILETAB_H
#define FILETAB_H

#include <QWidget>
#include <QUrl>
#include <directory.h>

namespace Ui {
    class FileTab;
}

struct FileTabPrivate;
class FileColumn;
class FileTab : public QWidget {
        Q_OBJECT

    public:
        explicit FileTab(QWidget* parent = nullptr);
        ~FileTab();

        enum ViewType {
            Columns,
            Trash
        };

        void setCurrentUrl(QUrl url);
        void setCurrentDir(DirectoryPtr directory);
        QUrl currentUrl();

        struct OpenFileButton {
            QString text;
            QIcon icon;
            std::function<void(QList<QUrl>)> activated;
            bool defaultAction = false;
        };

        struct ColumnAction {
            QString text;
            QString buttonText;
            std::function<void(DirectoryPtr)> activated;
        };

        struct Filter {
            bool isMimeFilter;
            QString filter;
        };

        void setFileTransfersSupported(bool supported);
        void setCanOpenProperties(bool canOpen);
        void setOpenFileButtons(QList<OpenFileButton> buttons);
        void setColumnActions(QList<ColumnAction> actions);
        void setFilters(QList<Filter> filters);

        FileColumn* currentColumn();
        FileColumn* lastColumn();

        QString tabTitle();

        void closeTab();

    signals:
        void currentUrlChanged(QUrl url);
        void tabClosed();
        void tabTitleChanged();
        void columnsChanged();
        void moveFiles(QList<QUrl> source, DirectoryPtr destination);
        void copyFiles(QList<QUrl> source, DirectoryPtr destination);
        void deletePermanently(QList<QUrl> filesToDelete);
        void openItemProperties(QUrl file);
        void burnDirectory(DirectoryPtr directory);

    private:
        Ui::FileTab* ui;
        FileTabPrivate* d;

        ViewType effectiveViewType();
};

#endif // FILETAB_H
