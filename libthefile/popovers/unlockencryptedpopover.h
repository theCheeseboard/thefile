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
#ifndef UNLOCKENCRYPTEDPOPOVER_H
#define UNLOCKENCRYPTEDPOPOVER_H

#include <DriveObjects/diskobject.h>
#include <QWidget>

namespace Ui {
    class UnlockEncryptedPopover;
}

struct UnlockEncryptedPopoverPrivate;
class UnlockEncryptedPopover : public QWidget {
        Q_OBJECT

    public:
        explicit UnlockEncryptedPopover(DiskObject* disk, QWidget* parent = nullptr);
        ~UnlockEncryptedPopover();

    signals:
        void reject();
        void accept(DiskObject* cleartext);

    private slots:
        void on_titleLabel_backButtonClicked();

        QCoro::Task<> on_okPasswordButton_clicked();

    private:
        Ui::UnlockEncryptedPopover* ui;
        UnlockEncryptedPopoverPrivate* d;
};

#endif // UNLOCKENCRYPTEDPOPOVER_H
