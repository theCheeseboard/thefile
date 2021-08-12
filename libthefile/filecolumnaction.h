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
#ifndef FILECOLUMNACTION_H
#define FILECOLUMNACTION_H

#include <QWidget>

namespace Ui {
    class FileColumnAction;
}

class FileColumnAction : public QWidget {
        Q_OBJECT

    public:
        explicit FileColumnAction(QWidget* parent = nullptr);
        ~FileColumnAction();

        void setText(QString text);
        void setButtonText(QString text);

    signals:
        void actionClicked();

    private slots:
        void on_actionButton_clicked();

    private:
        Ui::FileColumnAction* ui;
};

#endif // FILECOLUMNACTION_H
