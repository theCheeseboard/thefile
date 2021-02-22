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
#ifndef ITEMPROPERTIESPOPOVER_H
#define ITEMPROPERTIESPOPOVER_H

#include <QWidget>
#include <QRunnable>

namespace Ui {
    class ItemPropertiesPopover;
}

struct ItemPropertiesPopoverPrivate;
class ItemPropertiesPopover : public QWidget {
        Q_OBJECT

    public:
        explicit ItemPropertiesPopover(QUrl url, QWidget* parent = nullptr);
        ~ItemPropertiesPopover();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_detailsButton_toggled(bool checked);

        void on_permissionsButton_toggled(bool checked);

    signals:
        void done();

    private:
        Ui::ItemPropertiesPopover* ui;
        ItemPropertiesPopoverPrivate* d;
};

struct CountDirectorySizesRunnablePrivate;
class CountDirectorySizesRunnable : public QObject, public QRunnable {
        Q_OBJECT
    public:
        explicit CountDirectorySizesRunnable(QUrl url, ItemPropertiesPopover* parent);
        ~CountDirectorySizesRunnable();

    signals:
        void progress(quint64 items, quint64 bytes, bool done);

    private:
        CountDirectorySizesRunnablePrivate* d;

        // QRunnable interface
    public:
        void run();
};

#endif // ITEMPROPERTIESPOPOVER_H
