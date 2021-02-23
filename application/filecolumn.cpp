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
#include <tpopover.h>
#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>
#include <QScrollBar>
#include <MimeAssociations/mimeassociationmanager.h>
#include "hiddenfilesproxymodel.h"
#include "jobs/filetransferjob.h"
#include "popovers/deletepermanentlypopover.h"
#include "filecolumnaction.h"
#include "filecolumnfloater.h"
#include "filecolumnmanager.h"
#include "popovers/itempropertiespopover.h"
#include "popovers/burnpopover.h"
#include <driveobjectmanager.h>

struct FileColumnPrivate {
    FileColumnManager* manager;
    DirectoryPtr directory;
    QUrl selectedUrl;
    FileModel* model = nullptr;
    FileDelegate* delegate;
    HiddenFilesProxyModel* proxy;

    FileColumnFloater* floater;
    tVariantAnimation* floaterAnim;

    QList<FileColumnAction*> actions;

    bool listenToSelection = true;
    bool isFloaterVisible = true;
};

FileColumn::FileColumn(DirectoryPtr directory, FileColumnManager* manager, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileColumn) {
    ui->setupUi(this);

    d = new FileColumnPrivate();
    d->directory = directory;
    d->manager = manager;

    this->setAcceptDrops(true);

    d->delegate = new FileDelegate(this);

    d->proxy = new HiddenFilesProxyModel(this);
    ui->folderView->setModel(d->proxy);
    ui->folderView->setItemDelegate(d->delegate);
    ui->folderView->installEventFilter(this);

    QShortcut* copyShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_C), this);
    connect(copyShortcut, &QShortcut::activated, this, &FileColumn::copy);
    QShortcut* cutShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_X), this);
    connect(cutShortcut, &QShortcut::activated, this, &FileColumn::cut);
    QShortcut* pasteShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_V), this);
    connect(pasteShortcut, &QShortcut::activated, this, &FileColumn::paste);
    QShortcut* deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(deleteShortcut, &QShortcut::activated, this, [ = ] {
        if (d->directory->url().scheme() == "trash") {
            //Delete Permanently
            deleteFile();
        } else {
            moveToTrash();
        }
    });
    QShortcut* deletePermanentlyShortcut = new QShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete), this);
    connect(deletePermanentlyShortcut, &QShortcut::activated, this, &FileColumn::deleteFile);

    d->floater = new FileColumnFloater(this);
    d->floater->setVisible(false);

    d->floaterAnim = new tVariantAnimation(this);
    d->floaterAnim->setDuration(500);
    d->floaterAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->floaterAnim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        d->floater->setGeometry(value.toRect());
        ui->folderScrollerWidget->layout()->setContentsMargins(0, 0, 0, this->height() - value.toRect().top() + SC_DPI(9));
    });
    hideFloater();

    ui->folderView->setFixedWidth(this->width());
    ui->folderScroller->viewport()->installEventFilter(this);
    connect(d->manager, &FileColumnManager::currentChanged, this, &FileColumn::updateFloater);

    reload();
}

FileColumn::~FileColumn() {
    delete ui;
    delete d;
}

void FileColumn::setDirectory(DirectoryPtr directory) {
    if (d->directory->url() == directory->url()) return;
    d->directory = directory;

    emit directoryChanged();
    reload();
}

void FileColumn::setSelected(QUrl url) {
    d->selectedUrl = url;
    ensureUrlSelected();
}

QString FileColumn::columnTitle() {
    if (d->directory->url().path() == "/") {
        if (d->directory->url().scheme() == "file") return tr("Root");
        if (d->directory->url().scheme() == "trash") return tr("Trash");
        return d->directory->url().scheme();
    } else {
        if (d->directory->url().scheme() == "file") {
            if (QDir(d->directory->url().path()) == QDir::home()) return tr("Home");
        }
        return QFileInfo(QFileInfo(d->directory->url().path()).canonicalFilePath()).fileName();
    }
}

void FileColumn::cut() {
    QStringList clipboardData;
    clipboardData.append("x-special/nautilus-clipboard");
    clipboardData.append("cut");

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    QList<QUrl> urls;
    for (QModelIndex index : qAsConst(sel)) {
        urls.append(index.data(FileModel::UrlRole).toUrl());
    }
    clipboardData.append(QUrl::toStringList(urls));

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("text/plain", clipboardData.join("\n").toUtf8());
    mimeData->setData("COMPOUND_TEXT", clipboardData.join("\n").toUtf8());
    mimeData->setData("text/plain;charset=utf-8", clipboardData.join("\n").toUtf8());
    mimeData->setData("application/x-thesuite-thefile-clipboardoperation", "cut");
    mimeData->setUrls(urls);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileColumn::copy() {
    QStringList clipboardData;
    clipboardData.append("x-special/nautilus-clipboard");
    clipboardData.append("copy");

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    QList<QUrl> urls;
    for (QModelIndex index : qAsConst(sel)) {
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
        FileTransferJob::TransferType transferType = FileTransferJob::Copy;
        if (data->hasFormat("application/x-thesuite-thefile-clipboardoperation")) {
            QString type = data->data("application/x-thesuite-thefile-clipboardoperation");
            if (type == "cut") transferType = FileTransferJob::Move;
        }

        //Prepare a copy job
        FileTransferJob* job = new FileTransferJob(transferType, data->urls(), d->directory, this->window());
        tJobManager::trackJobDelayed(job);

        //Clear the clipboard after a cut operation
        if (transferType == FileTransferJob::Move) QApplication::clipboard()->clear();
    }
}

void FileColumn::newFolder() {
    bool ok;
    QString folderName = QInputDialog::getText(this, tr("New Folder"), tr("Folder name"), QLineEdit::Normal, tr("New Folder"), &ok);
    if (ok) {
        d->directory->mkpath(folderName);
    }
}

void FileColumn::moveToTrash() {
    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    for (QModelIndex index : sel) {
        d->directory->trash(index.data(FileModel::PathSegmentRole).toString());
    }

    tToast* toast = new tToast(this->window());
    toast->setTitle(tr("Trash"));
    toast->setText(tr("Moved %n items to the trash", nullptr, sel.count()));
    connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
    toast->show(this->window());

    emit navigate(d->directory);
}

void FileColumn::deleteFile() {
    QList<QUrl> urlsToDelete;

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    for (QModelIndex index : sel) {
        urlsToDelete.append(index.data(FileModel::UrlRole).toUrl());
    }

    DeletePermanentlyPopover* jp = new DeletePermanentlyPopover(urlsToDelete);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &DeletePermanentlyPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
    popover->show(this->window());

    emit navigate(d->directory);
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
            d->directory->move(item.data(FileModel::PathSegmentRole).toString(), newUrl);
        }
    }
}

void FileColumn::reload() {
    d->model = new FileModel(d->directory);
    d->proxy->setSourceModel(d->model);
    connect(d->model, &FileModel::modelAboutToBeReset, this, [ = ] {
        ui->folderView->selectionModel()->blockSignals(true);
    });
    connect(d->model, &FileModel::modelReset, this, &FileColumn::updateItems);
    connect(d->model, &FileModel::modelReset, this, &FileColumn::ensureUrlSelected);
    connect(d->model, &FileModel::modelReset, this, [ = ] {
        ui->folderView->selectionModel()->blockSignals(false);
        ui->folderView->setFixedHeight(ui->folderView->sizeHintForRow(0) * d->proxy->rowCount());
    });
    updateItems();
    ui->folderView->setFixedHeight(ui->folderView->sizeHintForRow(0) * d->proxy->rowCount());

    ui->folderNameLabel->setText(columnTitle());
    connect(ui->folderView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ = ] {
        updateFloater();

        if (!d->listenToSelection) return;
        d->floater->setIndices(ui->folderView->selectionModel()->selectedIndexes());
        if (ui->folderView->selectionModel()->selectedIndexes().count() == 1) {
            emit navigate(ResourceManager::directoryForUrl(ui->folderView->selectionModel()->selectedIndexes().at(0).data(FileModel::UrlRole).toUrl()));
        } else if (ui->folderView->selectionModel()->selectedIndexes().isEmpty()) {
            emit navigate(d->directory);
        }
    });
}

void FileColumn::updateItems() {
    QString error = d->model->currentError();
    if (d->model->isFile()) {
        ResourceManager::parentDirectoryForUrl(d->directory->url())->fileInformation(d->directory->url().fileName())->then([ = ] (Directory::FileInformation fileInfo) {
            ui->fileIconLabel->setPixmap(fileInfo.icon.pixmap(SC_DPI_T(QSize(128, 128), QSize)));
            ui->filenameLabel->setText(fileInfo.name);

            QStringList fileInfoText;
            fileInfoText.append(QLocale().formattedDataSize(fileInfo.size));
            ui->fileInfoLabel->setText(fileInfoText.join(" · "));

            ui->stackedWidget->setCurrentWidget(ui->filePage);
        });
    } else if (error.isEmpty()) {
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

    //Add actions
    for (FileColumnAction* action : d->actions) {
        ui->actionsLayout->removeWidget(action);
        action->deleteLater();
    }
    d->actions.clear();

    if (d->directory->url().scheme() == "trash") {
        FileColumnAction* trashAction = new FileColumnAction(this);
        trashAction->setText(tr("Trash"));
        trashAction->setButtonText(tr("Empty Trash"));
        connect(trashAction, &FileColumnAction::actionClicked, this, [ = ] {
            ui->folderView->selectAll();
            deleteFile();
        });
        ui->actionsLayout->addWidget(trashAction);
        d->actions.append(trashAction);
    }
}

void FileColumn::showFloater() {
    if (d->isFloaterVisible) return;
    d->floaterAnim->setStartValue(d->floater->geometry());

    QRect endGeometry;
    endGeometry.setHeight(d->floater->sizeHint().height());
    endGeometry.moveBottom(this->height() - SC_DPI(9));
    endGeometry.setLeft(SC_DPI(9));
    endGeometry.setRight(this->width() - SC_DPI(9));
    d->floaterAnim->setEndValue(endGeometry);
    d->floaterAnim->start();

    d->floater->setVisible(true);
    d->isFloaterVisible = true;
}

void FileColumn::hideFloater() {
    if (!d->isFloaterVisible) return;
    d->floaterAnim->setStartValue(d->floater->geometry());

    QRect endGeometry;
    endGeometry.setHeight(d->floater->sizeHint().height());
    endGeometry.moveTop(this->height() + SC_DPI(9));
    endGeometry.setLeft(SC_DPI(9));
    endGeometry.setRight(this->width() - SC_DPI(9));
    d->floaterAnim->setEndValue(endGeometry);
    d->floaterAnim->start();

    d->isFloaterVisible = false;
}

void FileColumn::updateFloater() {
    if (ui->folderView->selectionModel()->selectedIndexes().isEmpty() || d->manager->current() != this) {
        hideFloater();
    } else {
        showFloater();
    }
}

void FileColumn::addFolderMenuItems(QMenu* menu) {
    if (d->model) {
        if (d->directory->url().scheme() == "trash") {

        } else {
            if (d->model->currentError().isEmpty() || d->model->currentError() == QStringLiteral("error.no-items")) {
                menu->addSection(tr("For this folder"));
                menu->addAction(QIcon::fromTheme("folder-new"), tr("New Folder"), this, &FileColumn::newFolder);
                menu->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"), this, &FileColumn::paste);
            }
        }
    }
}

void FileColumn::ensureUrlSelected() {
    d->listenToSelection = false;
    if (d->selectedUrl.isValid()) {
        for (int i = 0; i < ui->folderView->model()->rowCount(); i++) {
            QModelIndex index = ui->folderView->model()->index(i, 0);
            QUrl checkUrl = index.data(FileModel::UrlRole).toUrl();
            if (d->selectedUrl.scheme() == checkUrl.scheme() && d->selectedUrl.host() == checkUrl.host() && QDir::cleanPath(d->selectedUrl.path()) == QDir::cleanPath(checkUrl.path()) && d->selectedUrl.query() == checkUrl.query()) {
                ui->folderView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            }
        }
    } else {
        ui->folderView->clearSelection();
    }
    d->listenToSelection = true;
}

void FileColumn::on_folderErrorPage_customContextMenuRequested(const QPoint& pos) {
    QMenu* menu = new QMenu(this);
    addFolderMenuItems(menu);
    menu->popup(ui->folderView->mapToGlobal(pos));
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
}

void FileColumn::on_folderView_doubleClicked(const QModelIndex& index) {
    QUrl url = index.data(FileModel::UrlRole).toUrl();
    DirectoryPtr dir = ResourceManager::directoryForUrl(url);
    if (dir) {
        dir->exists()->then([ = ](bool exists) {
            if (!exists) {
                QDesktopServices::openUrl(url);
            }
        });
    } else {
        QDesktopServices::openUrl(url);
    }
}

bool FileColumn::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->folderScroller->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
//            QMouseEvent* e = static_cast<QMouseEvent*>(event);
//            if (!ui->folderView->indexAt(e->pos()).isValid()) {
            ui->folderView->selectionModel()->clear();
            ui->folderView->selectionModel()->clear();
            d->manager->setCurrent(this);
            return true;
//            }
        }
    } else if (watched == ui->folderView) {
        if (event->type() == QEvent::FocusIn) {
            d->manager->setCurrent(this);
        }
    }
    return false;
}

void FileColumn::resizeEvent(QResizeEvent* event) {
    QRect endGeometry;
    endGeometry.setHeight(d->floater->sizeHint().height());
    if (d->isFloaterVisible) {
        endGeometry.moveBottom(this->height() - SC_DPI(9));
    } else {
        endGeometry.moveTop(this->height() + SC_DPI(9));
    }
    endGeometry.setLeft(SC_DPI(9));
    endGeometry.setRight(this->width() - SC_DPI(9));
    d->floater->setGeometry(endGeometry);
    d->floaterAnim->setEndValue(endGeometry);

    ui->folderView->setFixedWidth(this->width());
}

void FileColumn::on_openFileButton_clicked() {
    QDesktopServices::openUrl(d->directory->url());
}


void FileColumn::focusInEvent(QFocusEvent* event) {
    d->manager->setCurrent(this);
}

void FileColumn::dragEnterEvent(QDragEnterEvent* event) {
    if (ui->stackedWidget->currentWidget() == ui->filePage) {
        event->setDropAction(Qt::IgnoreAction);
        return;
    }

    event->acceptProposedAction();
}

void FileColumn::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    tDebug("FileColumn") << mimeData->formats();
    QModelIndex index = ui->folderView->indexAt(ui->folderView->mapFrom(this, event->pos()));

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        QMenu* menu = new QMenu();
        if (index.isValid()) {
            QUrl url = index.data(FileModel::UrlRole).toUrl();
            DirectoryPtr dir = ResourceManager::directoryForUrl(url);
            if (dir && dir->exists()->await().result) {
                menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
                menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy In"), this, [ = ] {
                    FileTransferJob* job = new FileTransferJob(FileTransferJob::Copy, urls, dir, this->window());
                    tJobManager::trackJobDelayed(job);
                });
                menu->addAction(QIcon::fromTheme("edit-cut"), tr("Move In"), this, [ = ] {
                    FileTransferJob* job = new FileTransferJob(FileTransferJob::Move, urls, dir, this->window());
                    tJobManager::trackJobDelayed(job);
                });
            }
        }

        menu->addSection(tr("For this folder"));
        menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy Here"), this, [ = ] {
            FileTransferJob* job = new FileTransferJob(FileTransferJob::Copy, urls, d->directory, this->window());
            tJobManager::trackJobDelayed(job);
        });
        menu->addAction(QIcon::fromTheme("edit-cut"), tr("Move Here"), this, [ = ] {
            FileTransferJob* job = new FileTransferJob(FileTransferJob::Move, urls, d->directory, this->window());
            tJobManager::trackJobDelayed(job);
        });

        menu->popup(this->mapToGlobal(event->pos()));
        connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    }
    event->setDropAction(Qt::CopyAction);
}

void FileColumn::on_folderScroller_customContextMenuRequested(const QPoint& pos) {
    QMenu* menu = new QMenu(this);

    QModelIndexList sel = ui->folderView->selectionModel()->selectedIndexes();
    if (sel.count() > 0) {
        if (sel.count() == 1) {
            menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(sel.first().data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
            QUrl url = sel.first().data(FileModel::UrlRole).toUrl();
            if (d->directory->isFile(sel.first().data(FileModel::PathSegmentRole).toString())) {
                menu->addAction(QIcon::fromTheme("document-open"), tr("Open"), this, [ = ] {
                    QDesktopServices::openUrl(url);
                });

                QMimeDatabase db;
                QMimeType mimeType = db.mimeTypeForFile(url.toLocalFile());

                QMenu* otherApps = new QMenu();
                otherApps->setTitle(tr("Open With..."));

                QList<ApplicationPointer> apps = MimeAssociationManager::applicationsForMimeType(mimeType.name());
                for (ApplicationPointer app : qAsConst(apps)) {
                    otherApps->addAction(QIcon::fromTheme(app->getProperty("Icon").toString()), app->getProperty("Name").toString(), this, [ = ] {

                        QMap<QString, QString> launchArgs;
                        launchArgs.insert("%u", url.toString());
                        launchArgs.insert("%U", url.toString());
                        launchArgs.insert("%f", url.toLocalFile());
                        launchArgs.insert("%F", url.toLocalFile());
                        app->launch(launchArgs);
                    });
                }

                otherApps->addSeparator();
                otherApps->addAction(tr("Another app..."), this, [ = ] {
                    QProcess::startDetached("xdg-open", {"--force-prompt", url.toString()});
                });

                menu->addMenu(otherApps);
                menu->addSeparator();
            }
        } else if (sel.count() > 1) {
            menu->addSection(tr("For %n items", nullptr, sel.count()));
        }

        menu->addAction(QIcon::fromTheme("edit-cut"), tr("Cut"), this, &FileColumn::cut);
        menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this, &FileColumn::copy);
        if (d->directory->url().scheme() == "trash") {
            menu->addAction(QIcon::fromTheme("trash-restore"), tr("Put Back"), this, [ = ] {
                for (QModelIndex index : sel) {
                    QUrl url = index.data(FileModel::UrlRole).toUrl();
                    QVariant restorePath = d->directory->special("restorePath", {{"url", url}});
                    if (restorePath.isValid()) {
                        QUrl dest = restorePath.toUrl();

                        //Prepare a move job
                        FileTransferJob* job = new FileTransferJob(FileTransferJob::Move, {url}, ResourceManager::parentDirectoryForUrl(dest), this->window());
                        tJobManager::trackJobDelayed(job);
                    }
                }
            });
            menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete Permanently"), this, &FileColumn::deleteFile);
        } else {
            menu->addAction(QIcon::fromTheme("edit-delete"), tr("Move to Trash"), this, [ = ] {
                if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
                    deleteFile();
                } else {
                    moveToTrash();
                }
            });
            menu->addAction(QIcon::fromTheme("edit-rename"), tr("Rename"), this, &FileColumn::rename);
        }
    }

    if (sel.count() == 1) {
        QUrl url = sel.first().data(FileModel::UrlRole).toUrl();

        if (url.scheme() == "file") {
            menu->addAction(QIcon::fromTheme("configure"), tr("Properties"), this, [ = ] {
                ItemPropertiesPopover* jp = new ItemPropertiesPopover(url);
                tPopover* popover = new tPopover(jp);
                popover->setPopoverWidth(SC_DPI(-200));
                popover->setPopoverSide(tPopover::Bottom);
                connect(jp, &ItemPropertiesPopover::done, popover, &tPopover::dismiss);
                connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
                connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
                popover->show(this->window());
            });
        }

        //TODO: Asynchronous
        DirectoryPtr dir = ResourceManager::directoryForUrl(url);
        if (dir && dir->exists()->await().result) {
            if (!DriveObjectManager::opticalDisks().isEmpty()) {
                menu->addSeparator();
                menu->addAction(QIcon::fromTheme("tools-media-optical-burn"), tr("Burn Contents"), this, [ = ] {
                    BurnPopover* jp = new BurnPopover(dir);
                    tPopover* popover = new tPopover(jp);
                    popover->setPopoverWidth(SC_DPI(-200));
                    popover->setPopoverSide(tPopover::Bottom);
                    connect(jp, &BurnPopover::done, popover, &tPopover::dismiss);
                    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
                    connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
                    popover->show(this->window());
                });
            }
        }
    }

    addFolderMenuItems(menu);

    menu->popup(ui->folderScroller->mapToGlobal(pos));
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
}
