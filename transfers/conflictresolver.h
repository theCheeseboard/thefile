/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
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
#ifndef CONFLICTRESOLVER_H
#define CONFLICTRESOLVER_H

#include <QWidget>
#include <QSharedPointer>
#include "transferpane.h"

namespace Ui {
    class ConflictResolver;
}

class ConflictResolver : public QWidget
{
        Q_OBJECT

    public:
        explicit ConflictResolver(QSharedPointer<FileConflict> conflict, QWidget *parent = nullptr);
        ~ConflictResolver();

        void setResolution(FileConflict::Resolution resolution);

    private slots:
        void on_replaceButton_toggled(bool checked);

        void on_skipButton_toggled(bool checked);

        void on_keepBothButton_toggled(bool checked);

    private:
        Ui::ConflictResolver *ui;

        QSharedPointer<FileConflict> conflict;
};

#endif // CONFLICTRESOLVER_H
