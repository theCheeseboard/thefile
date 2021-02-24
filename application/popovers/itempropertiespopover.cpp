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
#include "itempropertiespopover.h"
#include "ui_itempropertiespopover.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrl>
#include <QDateTime>
#include <QStorageInfo>
#include <tpromise.h>

struct ItemPropertiesPopoverPrivate {
    QUrl url;
};

struct CountDirectorySizesRunnablePrivate {
    QPointer<ItemPropertiesPopover> parentPopover;
    QUrl url;
};

ItemPropertiesPopover::ItemPropertiesPopover(QUrl url, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ItemPropertiesPopover) {
    ui->setupUi(this);
    d = new ItemPropertiesPopoverPrivate();
    d->url = url;

    ui->titleLabel->setBackButtonShown(true);
    ui->tabSwitcher->setFixedWidth(SC_DPI(600));
    ui->stackedWidget->setFixedWidth(SC_DPI(600));

    ui->filesizeSpinner->setFixedSize(SC_DPI_T(QSize(16, 16), QSize));

    if (url.scheme() == "file") {
        QMimeDatabase db;

        QFileInfo file(url.toLocalFile());
        QStorageInfo storage(file.dir());
        QMimeType mimeType = db.mimeTypeForFile(file);
        ui->filenameLabel->setText(file.fileName());
        ui->filetypeLabel->setText(mimeType.comment());
        ui->filemimeLabel->setText(mimeType.name());
        ui->freeSpaceLabel->setText(QLocale().formattedDataSize(storage.bytesAvailable()));
        ui->fileAccessedLabel->setText(QLocale().toString(file.lastRead()));
        ui->fileModifiedLabel->setText(QLocale().toString(file.lastModified()));

        if (file.isDir()) {
            ui->filesizeLabel->setText(tr("Counting..."));
            ui->filesizeBytesLabel->setText(tr("Counting..."));

            //Start counting the contents of the directory
            CountDirectorySizesRunnable* countRunnable = new CountDirectorySizesRunnable(url, this);
            connect(countRunnable, &CountDirectorySizesRunnable::progress, this, [ = ](quint64 items, quint64 bytes, bool done) {
                QStringList parts;
                parts.append(tr("%n items", nullptr, items));
                parts.append(QLocale().formattedDataSize(bytes));
                ui->filesizeLabel->setText(parts.join(" · "));
                ui->filesizeBytesLabel->setText(tr("%1 bytes").arg(QLocale().toString(bytes)));
                ui->filesizeSpinner->setVisible(!done);
            });
            QThreadPool::globalInstance()->start(countRunnable);
        } else {
            ui->filesizeLabel->setText(QLocale().formattedDataSize(file.size()));
            ui->filesizeBytesLabel->setText(tr("%1 bytes").arg(QLocale().toString(file.size())));
            ui->filesizeSpinner->setVisible(false);
        }

        //Permissions
        QSignalBlocker blockers[] = {
            QSignalBlocker(ui->ownerReadBox),
            QSignalBlocker(ui->ownerWriteBox),
            QSignalBlocker(ui->groupReadBox),
            QSignalBlocker(ui->groupWriteBox),
            QSignalBlocker(ui->otherReadBox),
            QSignalBlocker(ui->otherWriteBox),
            QSignalBlocker(ui->executableBox)
        };
        ui->ownerReadBox->setChecked(file.permission(QFile::ReadOwner));
        ui->ownerWriteBox->setChecked(file.permission(QFile::WriteOwner));
        ui->groupReadBox->setChecked(file.permission(QFile::ReadGroup));
        ui->groupWriteBox->setChecked(file.permission(QFile::WriteGroup));
        ui->otherReadBox->setChecked(file.permission(QFile::ReadOther));
        ui->otherWriteBox->setChecked(file.permission(QFile::WriteOther));
        ui->executableBox->setChecked(file.permission(QFile::ExeOwner));
        ui->fileOwnerLabel->setText(file.owner());
        ui->fileGroupLabel->setText(file.group());
    }
}

ItemPropertiesPopover::~ItemPropertiesPopover() {
    delete ui;
    delete d;
}

void ItemPropertiesPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void ItemPropertiesPopover::on_detailsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->detailsPage);
    }
}

void ItemPropertiesPopover::on_permissionsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->permissionsPage);
    }
}

void ItemPropertiesPopover::setPermissions() {
    QFile::Permissions permissions;
    if (ui->ownerReadBox->isChecked()) permissions |= QFile::ReadOwner;
    if (ui->ownerWriteBox->isChecked()) permissions |= QFile::WriteOwner;
    if (ui->groupReadBox->isChecked()) permissions |= QFile::ReadGroup;
    if (ui->groupWriteBox->isChecked()) permissions |= QFile::WriteGroup;
    if (ui->otherReadBox->isChecked()) permissions |= QFile::ReadOther;
    if (ui->otherWriteBox->isChecked()) permissions |= QFile::WriteOther;
    if (ui->executableBox->isChecked()) permissions |= QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther;
    QFile::setPermissions(d->url.toLocalFile(), permissions);
}

CountDirectorySizesRunnable::CountDirectorySizesRunnable(QUrl url, ItemPropertiesPopover* parent) : QObject(nullptr), QRunnable() {
    d = new CountDirectorySizesRunnablePrivate();
    d->parentPopover = parent;
    d->url = url;
}

CountDirectorySizesRunnable::~CountDirectorySizesRunnable() {
    delete d;
}

void CountDirectorySizesRunnable::run() {
    quint64 count = 0;
    quint64 bytes = 0;

    QDirIterator iterator(d->url.toLocalFile(), QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        if (!d->parentPopover) break;
        iterator.next();
        count++;
        bytes += iterator.fileInfo().size();

        emit progress(count, bytes, false);
    }

    emit progress(count, bytes, true);
}

void ItemPropertiesPopover::on_ownerReadBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_ownerWriteBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_groupReadBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_groupWriteBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_otherReadBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_otherWriteBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}

void ItemPropertiesPopover::on_executableBox_toggled(bool checked) {
    Q_UNUSED(checked)
    setPermissions();
}
