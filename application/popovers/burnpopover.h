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
#ifndef BURNPOPOVER_H
#define BURNPOPOVER_H

#include <QWidget>
#include <directory.h>

namespace Ui {
    class BurnPopover;
}

struct BurnPopoverPrivate;
class BurnPopover : public QWidget {
        Q_OBJECT

    public:
        explicit BurnPopover(DirectoryPtr dir, QWidget* parent = nullptr);
        ~BurnPopover();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_discNameEdit_textChanged(const QString& arg1);

        void on_burnerBox_currentIndexChanged(int index);

        void on_burnButton_clicked();

        void on_doBurnButton_clicked();

        void on_titleLabel_2_backButtonClicked();

    signals:
        void done();

    private:
        Ui::BurnPopover* ui;
        BurnPopoverPrivate* d;

        void updateBurner();
        void performBurn();
};

#endif // BURNPOPOVER_H
