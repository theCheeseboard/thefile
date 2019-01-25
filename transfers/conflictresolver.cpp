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
#include "conflictresolver.h"
#include "ui_conflictresolver.h"

ConflictResolver::ConflictResolver(QSharedPointer<FileConflict> conflict, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConflictResolver)
{
    ui->setupUi(this);

    this->conflict = conflict;

    QFileInfo oldFile(conflict->file);
    QFileInfo newFile(conflict->conflictingFile);
    switch (conflict->nature) {
        case FileConflict::Conflict:
            ui->filenameLabel->setText(oldFile.fileName());
            ui->iconLabel->setVisible(false);
            break;
        case FileConflict::Warning:
            ui->filenameLabel->setText(conflict->explanation);
            ui->buttonsBox->setVisible(false);
            ui->iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
            break;
        case FileConflict::Error:
            ui->filenameLabel->setText(conflict->explanation);
            ui->replaceButton->setVisible(false);
            ui->keepBothButton->setVisible(false);
            ui->iconLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(16, 16));
            break;
    }

    ui->oldFileName->setText(oldFile.fileName());
    ui->newFileName->setText(newFile.fileName());
    ui->oldDir->setText(tr("Directory: %1").arg(oldFile.dir().dirName()));
    ui->newDir->setText(tr("Directory: %1").arg(newFile.dir().dirName()));

    if (oldFile.isFile()) {
        ui->oldSize->setText(tr("Size: %1").arg(QLocale().formattedDataSize(oldFile.size())));
    } else {
        ui->oldSize->setText(tr("Directory"));
    }

    if (newFile.isFile()) {
        ui->newSize->setText(tr("Size: %1").arg(QLocale().formattedDataSize(newFile.size())));
    } else {
        ui->newSize->setText(tr("Directory"));
    }

    ui->arrowRight->setPixmap(QIcon::fromTheme("arrow-right").pixmap(16, 16));
}

ConflictResolver::~ConflictResolver()
{
    delete ui;
}

void ConflictResolver::on_replaceButton_toggled(bool checked)
{
    if (checked) {
        this->conflict->resolution = FileConflict::Overwrite;
    }
}

void ConflictResolver::on_skipButton_toggled(bool checked)
{
    if (checked) {
        this->conflict->resolution = FileConflict::Skip;
    }
}

void ConflictResolver::on_keepBothButton_toggled(bool checked)
{
    if (checked) {
        this->conflict->resolution = FileConflict::Rename;
        this->conflict->newName = this->conflict->file + "(2)";
    }
}

void ConflictResolver::setResolution(FileConflict::Resolution resolution) {
    switch (resolution) {
        case FileConflict::Skip:
            if (ui->skipButton->isVisible() && ui->skipButton->isEnabled()) ui->skipButton->setChecked(true);
            break;
        case FileConflict::Overwrite:
            if (ui->replaceButton->isVisible() && ui->replaceButton->isEnabled()) ui->replaceButton->setChecked(true);
            break;
        case FileConflict::Rename:
            if (ui->keepBothButton->isVisible() && ui->keepBothButton->isEnabled()) ui->keepBothButton->setChecked(true);
            break;
    }
}
