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
#ifndef FILECOLUMNFLOATER_H
#define FILECOLUMNFLOATER_H

#include "filecolumn.h"
#include <QModelIndexList>

namespace Ui {
    class FileColumnFloater;
}

struct FileColumnFloaterPrivate;
class FileColumnFloater : public QWidget {
        Q_OBJECT

    public:
        explicit FileColumnFloater(FileColumn* parent = nullptr);
        ~FileColumnFloater();

        void setIndices(QModelIndexList indices);

    private slots:
        void on_cutButton_clicked();

        void on_copyButton_clicked();

        void on_actionsButton_pressed();

    private:
        Ui::FileColumnFloater* ui;
        FileColumnFloaterPrivate* d;

        void mousePressEvent(QMouseEvent* event);

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // FILECOLUMNFLOATER_H
