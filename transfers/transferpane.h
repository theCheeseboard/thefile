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
#ifndef TRANSFERPANE_H
#define TRANSFERPANE_H

#include <QFrame>
#include "transferengine.h"

class ConflictResolver;

namespace Ui {
    class TransferPane;
}

class TransferPane : public QFrame
{
        Q_OBJECT

    public:
        explicit TransferPane(TransferObject* transfer, QWidget *parent = nullptr);
        ~TransferPane();

        QSize sizeHint() const;

        bool needExtraHeight();

    public slots:
        void setActionLabelText(QString text);

        void resolveConflicts(FileConflictList conflicts, bool hasNonSimpleConflicts);

        void progress(qulonglong value, qulonglong max);

        void resize();

    signals:
        void heightChanged();

        void cancelTransfer();

        void conflictsResolved(FileConflictList resolutions);

    private slots:
        void on_replaceAllConflicts_clicked();

        void on_skipAllConflicts_clicked();

        void on_cancelButton_clicked();

        void on_examineAllConflicts_clicked();

        void on_skipAllButton_clicked();

        void on_replaceAllButton_clicked();

    private:
        Ui::TransferPane *ui;

        TransferObject* transfer;
        QList<ConflictResolver*> resolvers;

        void resizeEvent(QResizeEvent* event);

        FileConflictList conflicts;
};

#endif // TRANSFERPANE_H
