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
#include "deletepermanentlypopover.h"
#include "ui_deletepermanentlypopover.h"

#include <resourcemanager.h>

struct DeletePermanentlyPopoverPrivate {
    QList<QUrl> filesToDelete;
};

DeletePermanentlyPopover::DeletePermanentlyPopover(QList<QUrl> filesToDelete, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DeletePermanentlyPopover) {
    ui->setupUi(this);
    d = new DeletePermanentlyPopoverPrivate();
    d->filesToDelete = filesToDelete;

    ui->titleLabel->setBackButtonShown(true);
    ui->deleteConfirmWidget->setFixedWidth(SC_DPI(600));
    ui->doDeleteButton->setProperty("type", "destructive");
    ui->doDeleteButton->setText(tr("Delete %n files", nullptr, filesToDelete.count()));
}

DeletePermanentlyPopover::~DeletePermanentlyPopover() {
    delete ui;
}

void DeletePermanentlyPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void DeletePermanentlyPopover::on_doDeleteButton_clicked() {
    for (QUrl url : qAsConst(d->filesToDelete)) {
        ResourceManager::parentDirectoryForUrl(url)->deleteFile(url.fileName());
    }
    emit done();
}
