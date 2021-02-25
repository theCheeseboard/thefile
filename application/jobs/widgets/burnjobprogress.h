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
#ifndef BURNJOBPROGRESS_H
#define BURNJOBPROGRESS_H

#include <QWidget>

namespace Ui {
    class BurnJobProgress;
}

struct BurnJobProgressPrivate;
class BurnJob;
class BurnJobProgress : public QWidget {
        Q_OBJECT

    public:
        explicit BurnJobProgress(BurnJob* job, QWidget* parent = nullptr);
        ~BurnJobProgress();

    private slots:
        void on_continueBurnButton_clicked();

        void on_doCancelButton_clicked();

        void on_cancelButton_clicked();

        void on_stackedWidget_currentChanged(int arg1);

    private:
        Ui::BurnJobProgress* ui;
        BurnJobProgressPrivate* d;

        void updateState();
};

#endif // BURNJOBPROGRESS_H
