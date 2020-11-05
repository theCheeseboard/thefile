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
#include "filecolumn.h"
#include "ui_filecolumn.h"

#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QInputDialog>
#include <QClipboard>
#include <resourcemanager.h>
#include <tlogger.h>
#include "filemodel.h"
#include <tjobmanager.h>
#include <ttoast.h>
#include <QMessageBox>
#include "hiddenfilesproxymodel.h"
#include "jobs/filetransferjob.h"

struct FileColumnPrivate {
    QUrl url;
    FileModel* model;
    HiddenFilesProxyModel* proxy;
};

FileColumn::FileColumn(QUrl url, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileColumn) {
    ui->setupUi(this);

    d = new FileColumnPrivate();
    d->url = url;

    d->proxy = new HiddenFilesProxyModel();
    ui->folderView->setModel(d->proxy);

    ui->folderView->setItemDelegate(new FileDelegate());

    reload();
}

FileColumn::~FileColumn() {
    delete ui;
    delete d;
}

void FileColumn::setUrl(QUrl url) {
    d->url = url;

    reload();
}

void FileColumn::setSelected(QUrl url) {
    if (url.isValid()) {
        for (int i = 0; i < ui->folderView->model()->rowCount(); i++) {
            QModelIndex index = ui->folderView->model()->index(i, 0);
            QUrl checkUrl = index.data(FileModel::UrlRole).toUrl();
            if (url.scheme() == checkUrl.scheme() && url.host() == checkUrl.host() && QDir::cleanPath(url.path()) == QDir::cleanPath(checkUrl.path())) {
                ui->folderView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            }
        }
    } else {
        ui->folderView->clearSelection();
    }
}

void FileColumn::copy() {
    QStringList clipboardData;
    clipboardData.append("x-special/nautilus-clipboard");
    clipboardData.append("copy");

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    QList<QUrl> urls;
    for (QModelIndex index : sel) {
        urls.append(index.data(FileModel::UrlRole).toUrl());
    }
    clipboardData.append(QUrl::toStringList(urls));

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("text/plain", clipboardData.join("\n").toUtf8());
    mimeData->setData("COMPOUND_TEXT", clipboardData.join("\n").toUtf8());
    mimeData->setData("text/plain;charset=utf-8", clipboardData.join("\n").toUtf8());
    mimeData->setData("application/x-thesuite-thefile-clipboardoperation", "copy");
    mimeData->setUrls(urls);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileColumn::paste() {
    const QMimeData* data = QApplication::clipboard()->mimeData();
    if (data->hasUrls()) {
        //Prepare a copy job
        FileTransferJob* job = new FileTransferJob(FileTransferJob::Copy, data->urls(), d->url, this->window());
        tJobManager::trackJobDelayed(job);
    }

    for (QString format : data->formats()) {
        tDebug("FileColumn") << format << " -> " << QString(data->data(format));
    }
}

void FileColumn::newFolder() {
    bool ok;
    QString folderName = QInputDialog::getText(this, tr("New Folder"), tr("Folder name"), QLineEdit::Normal, tr("New Folder"), &ok);
    if (ok) {
        ResourceManager::mkpath(d->url.resolved(folderName));
    }
}

void FileColumn::moveToTrash() {
    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    for (QModelIndex index : sel) {
        ResourceManager::trash(index.data(FileModel::UrlRole).toUrl());
    }

    tToast* toast = new tToast(this);
    toast->setTitle(tr("Trash"));
    toast->setText(tr("Moved %n items to the trash", nullptr, sel.count()));
    connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
    toast->show(this->window());
}

void FileColumn::deleteFile() {
    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    if (QMessageBox::warning(this, tr("Delete %n Files", nullptr, sel.count()), tr("Delete %n files from your device? This cannot be undone.", nullptr, sel.count()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        for (QModelIndex index : sel) {
            ResourceManager::deleteFile(index.data(FileModel::UrlRole).toUrl());
        }
    }
}

void FileColumn::rename() {
    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    if (sel.count() == 1) {
        QModelIndex item = sel.first();

        bool ok;
        QString newName = QInputDialog::getText(this, tr("Rename"), tr("Enter a new name"), QLineEdit::Normal, item.data(Qt::DisplayRole).toString(), &ok);
        if (ok) {
            QUrl oldUrl = item.data(FileModel::UrlRole).toUrl();
            QUrl newUrl = oldUrl.resolved(QUrl("./" + newName));
            ResourceManager::move(oldUrl, newUrl);
        }
    }
}

void FileColumn::reload() {
    d->model = new FileModel(d->url);
    connect(d->model, &FileModel::modelReset, this, &FileColumn::updateItems);
    updateItems();

    d->proxy->setSourceModel(d->model);
    if (d->url.path() == "/") {
        if (d->url.scheme() == "file") {
            ui->folderNameLabel->setText("/");
        } else {
            ui->folderNameLabel->setText(d->url.scheme());
        }
    } else {
        ui->folderNameLabel->setText(QFileInfo(QFileInfo(d->url.path()).canonicalFilePath()).fileName());
    }
    connect(ui->folderView->selectionModel(), &QItemSelectionModel::currentChanged, this, [ = ] {
        if (ui->folderView->currentIndex().isValid()) {
            emit navigate(ui->folderView->currentIndex().data(FileModel::UrlRole).toUrl());
        } else {
            emit navigate(d->url);
        }
    });
}

void FileColumn::updateItems() {
    QString error = d->model->currentError();
    if (error.isEmpty()) {
        ui->stackedWidget->setCurrentWidget(ui->folderPage);
    } else {
        QIcon icon;
        if (error == QStringLiteral("error.no-items")) {
            ui->folderErrorTitle->setText(tr("No items here!"));
            ui->folderErrorText->setText(tr("This folder is empty."));
            icon = QIcon(":/icons/folder-empty.svg");
        } else if (error == QStringLiteral("error.not-found")) {
            ui->folderErrorTitle->setText(tr("Not Found"));
            ui->folderErrorText->setText(tr("This folder doesn't exist."));
            icon = QIcon(":/icons/folder-unavailable.svg");
        } else if (error == QStringLiteral("error.permission-denied")) {
            ui->folderErrorTitle->setText(tr("Permission Denied"));
            ui->folderErrorText->setText(tr("Looks like you don't have permission to view this folder."));
            icon = QIcon(":/icons/folder-unavailable.svg");
        } else {
            ui->folderErrorTitle->setText(tr("Can't view this folder"));
            ui->folderErrorText->setText(tr("We can't show you the contents of this folder."));
            icon = QIcon(":/icons/folder-unavailable.svg");
        }
        QImage iconImage = icon.pixmap(SC_DPI_T(QSize(128, 128), QSize)).toImage();
        theLibsGlobal::tintImage(iconImage, this->palette().color(QPalette::WindowText));
        ui->folderErrorIcon->setPixmap(QPixmap::fromImage(iconImage));
        ui->stackedWidget->setCurrentWidget(ui->folderErrorPage);
    }
}

void FileColumn::on_folderView_customContextMenuRequested(const QPoint& pos) {
    QMenu* menu = new QMenu(this);

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    if (sel.count() > 0) {
        if (sel.count() == 1) {
            menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(sel.first().data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
        } else if (sel.count() > 1) {
            menu->addSection(tr("For %n items", nullptr, sel.count()));
        }

        menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FileColumn::copy);
        QAction* deleteAction = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Move to Trash"), this, [ = ] {
            if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
                deleteFile();
            } else {
                moveToTrash();
            }
        });
        menu->addAction(QIcon::fromTheme("edit-remane"), tr("Rename"), this, &FileColumn::rename);
    }

    menu->addSection(tr("For this folder"));
    menu->addAction(QIcon::fromTheme("folder-new"), tr("New Folder"), this, &FileColumn::newFolder);
    menu->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"), this, &FileColumn::paste);

    menu->popup(ui->folderView->mapToGlobal(pos));
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
}
