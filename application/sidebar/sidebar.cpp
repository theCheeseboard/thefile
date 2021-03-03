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
#include "sidebar.h"
#include "ui_sidebar.h"

#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <driveobjectmanager.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/filesysteminterface.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/encryptedinterface.h>
#include <QPainter>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <tmessagebox.h>
#include <tlogger.h>
#include <directory.h>
#include <tjobmanager.h>
#include <resourcemanager.h>
#include <tpopover.h>
#include "popovers/unlockencryptedpopover.h"
#include "jobs/filetransferjob.h"
#include "devicesmodel.h"

struct SidebarPrivate {
    DevicesModel* devicesModel;
};

Sidebar::Sidebar(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Sidebar) {
    ui->setupUi(this);
    d = new SidebarPrivate();

    struct Place {
        QString name;
        QIcon icon;
        QUrl location;
    };

    for (const Place& place : QList<Place>({
    {tr("Home"), QIcon::fromTheme("go-home"), QUrl::fromLocalFile(QDir::homePath())},
        {tr("Documents"), QIcon::fromTheme("folder-documents"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))},
        {tr("Downloads"), QIcon::fromTheme("folder-downloads"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))},
        {tr("Music"), QIcon::fromTheme("folder-music"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MusicLocation))},
        {tr("Pictures"), QIcon::fromTheme("folder-pictures"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))},
        {tr("Videos"), QIcon::fromTheme("folder-videos"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))},
        {tr("Root"), QIcon::fromTheme("folder-root"), QUrl::fromLocalFile(QDir::rootPath())},
        {tr("Trash"), QIcon::fromTheme("user-trash"), QUrl("trash:/")}
    })) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(place.name);
        item->setIcon(place.icon);
        item->setData(Qt::UserRole, place.location);
        ui->placesWidget->addItem(item);
    }

    ui->placesWidget->setItemDelegate(new SidebarDelegate());
    ui->placesWidget->setFixedHeight(ui->placesWidget->sizeHintForRow(0) * ui->placesWidget->count());

    d->devicesModel = new DevicesModel();
    ui->devicesView->setModel(d->devicesModel);
    ui->devicesView->setItemDelegate(new SidebarDelegate());
    connect(d->devicesModel, &DevicesModel::modelReset, this, [ = ] {
        ui->devicesView->setFixedHeight(ui->devicesView->sizeHintForRow(0) * d->devicesModel->rowCount());
    });
    ui->devicesView->setFixedHeight(ui->devicesView->sizeHintForRow(0) * d->devicesModel->rowCount());

    ui->placesWidget->setAcceptDrops(true);
    ui->placesWidget->installEventFilter(this);
}

Sidebar::~Sidebar() {
    delete d;
    delete ui;
}

void Sidebar::on_placesWidget_itemActivated(QListWidgetItem* item) {
    emit navigate(item->data(Qt::UserRole).toUrl());
}

void Sidebar::on_devicesView_activated(const QModelIndex& index) {
    //Ignore if we're trying to right click
    if (qApp->mouseButtons() & Qt::RightButton) return;

    //Mount and navigate to the item
    DiskObject* disk = index.data(DevicesModel::DiskObjectRole).value<DiskObject*>();
    mount(disk);
}


SidebarDelegate::SidebarDelegate(QObject* parent) : QStyledItemDelegate(parent) {

}

void SidebarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    //Offset everything by 6px
//    QStyleOptionViewItem newOption = option;
//    newOption.rect.adjust(SC_DPI(6), 0, 0, 0);
//    QStyledItemDelegate::paint(painter, newOption, index);

    painter->setPen(Qt::transparent);

    QPen textPen;
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.brush(QPalette::Highlight));
        textPen = option.palette.color(QPalette::HighlightedText);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.brush(QPalette::Base));
        textPen = option.palette.color(QPalette::WindowText);
    }
    painter->drawRect(option.rect);

    QRect contentRect = option.rect.adjusted(SC_DPI(9), 0, 0, 0);
    QRect iconRect = contentRect, textRect = contentRect;

    textRect.setHeight(contentRect.height() - SC_DPI(2));

    if (!index.data(Qt::DecorationRole).value<QIcon>().isNull()) {
        QSize iconSize = ((QAbstractItemView*) option.widget)->iconSize();
        if (!iconSize.isValid()) {
            iconSize = QSize(SC_DPI(16), SC_DPI(16));
        }

        iconRect.setSize(iconSize);
        QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        QImage iconImage = icon.pixmap(iconSize).toImage();
        iconRect.moveLeft(contentRect.left() + SC_DPI(2));
        iconRect.moveTop(contentRect.top() + (contentRect.height() / 2) - (iconRect.height() / 2));
        painter->drawImage(iconRect, iconImage);
        textRect.setLeft(iconRect.right() + SC_DPI(6));
    } else {
        textRect.setLeft(contentRect.left() + SC_DPI(6));
    }

    painter->setPen(textPen);
    painter->setFont(option.font);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

    if (index.data(DevicesModel::MountedRole).toBool()) {
        //Draw the mounted indicator
        QRect mountIndicator = option.rect;
        mountIndicator.setWidth(SC_DPI(6));
        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(0, 200, 0));
        painter->drawRect(mountIndicator);
    }
}

void Sidebar::on_devicesView_customContextMenuRequested(const QPoint& pos) {
    QModelIndex device = ui->devicesView->indexAt(pos);
    DiskObject* disk = device.data(DevicesModel::DiskObjectRole).value<DiskObject*>();
    FilesystemInterface* fs = disk->interface<FilesystemInterface>();
    BlockInterface* block = disk->interface<BlockInterface>();
    DriveInterface* drive = nullptr;
    if (block) drive = block->drive();

    QMenu* menu = new QMenu();
    menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(device.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
    if (fs) {
        if (fs->mountPoints().isEmpty()) {
            menu->addAction(QIcon::fromTheme("media-mount"), tr("Mount"), this, [ = ] {
                fs->mount();
            });
        } else {
            menu->addAction(QIcon::fromTheme("media-unmount"), tr("Unmount"), this, [ = ] {
                fs->unmount()->error([ = ](QString error) {
                    tMessageBox* box = new tMessageBox(this);
                    box->setWindowTitle(tr("Couldn't unmount"));
                    box->setText(tr("Unmounting the drive failed."));
                    box->setInformativeText(error);
                    connect(box, &tMessageBox::finished, box, &tMessageBox::deleteLater);
                    box->open();
                });
            });
        }
    }

    if (drive) {
        if (drive->ejectable()) menu->addAction(QIcon::fromTheme("media-eject"), tr("Eject"), this, [ = ] {
            drive->eject()->error([ = ](QString error) {
                tMessageBox* box = new tMessageBox(this);
                box->setWindowTitle(tr("Couldn't eject"));
                box->setText(tr("Ejecting the drive failed."));
                box->setInformativeText(error);
                connect(box, &tMessageBox::finished, box, &tMessageBox::deleteLater);
                box->open();
            });
        });
    }

    if (block && block->cryptoBackingDevice()) {
        menu->addAction(tr("Lock"), this, [ = ] {
            auto performLock = [ = ] {
                block->cryptoBackingDevice()->interface<EncryptedInterface>()->lock()->error([ = ](QString error) {
                    tMessageBox* box = new tMessageBox(this);
                    box->setWindowTitle(tr("Couldn't lock"));
                    box->setText(tr("Locking the device failed."));
                    box->setInformativeText(error);
                    connect(box, &tMessageBox::finished, box, &tMessageBox::deleteLater);
                    box->open();
                });
            };

            //Unmount the drive first, and then lock it
            if (fs->mountPoints().isEmpty()) {
                performLock();
            } else {
                fs->unmount()->then([ = ] {
                    performLock();
                })->error([ = ](QString error) {
                    tMessageBox* box = new tMessageBox(this);
                    box->setWindowTitle(tr("Couldn't unmount"));
                    box->setText(tr("Unmounting the drive failed."));
                    box->setInformativeText(error);
                    connect(box, &tMessageBox::finished, box, &tMessageBox::deleteLater);
                    box->open();
                });
            }
        });
    }
    menu->popup(ui->devicesView->mapToGlobal(pos));
}

bool Sidebar::eventFilter(QObject* watched, QEvent* event) {
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched);
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
        e->acceptProposedAction();
        return true;
    } else if (event->type() == QEvent::Drop) {
        QDropEvent* e = static_cast<QDropEvent*>(event);

        const QMimeData* mimeData = e->mimeData();
        tDebug("FileColumn") << mimeData->formats();
        QModelIndex index = view->indexAt(e->pos());

        if (mimeData->hasUrls()) {
            QList<QUrl> urls = mimeData->urls();
            if (index.isValid()) {
                QUrl url = index.data(Qt::UserRole).toUrl();
                DirectoryPtr dir = ResourceManager::directoryForUrl(url);
                if (url.scheme() == "trash") {
                    //Trash these items
                    for (const QUrl& url : qAsConst(urls)) {
                        ResourceManager::parentDirectoryForUrl(url)->trash(url.fileName());
                    }
                } else if (dir && dir->exists()->await().result) {
                    QMenu* menu = new QMenu();
                    menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
                    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy In"), this, [ = ] {
                        FileTransferJob* job = new FileTransferJob(FileTransferJob::Copy, urls, dir, this->window());
                        tJobManager::trackJobDelayed(job);
                    });
                    menu->addAction(QIcon::fromTheme("edit-cut"), tr("Move In"), this, [ = ] {
                        FileTransferJob* job = new FileTransferJob(FileTransferJob::Move, urls, dir, this->window());
                        tJobManager::trackJobDelayed(job);
                    });
                    menu->popup(this->mapToGlobal(e->pos()));
                    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
                }
            }
        }
        e->setDropAction(Qt::CopyAction);
        return true;
    }
    return false;
}

void Sidebar::mount(DiskObject* disk) {
    //Mount and navigate to the item
    FilesystemInterface* fs = disk->interface<FilesystemInterface>();
    EncryptedInterface* encrypted = disk->interface<EncryptedInterface>();
    if (fs) {
        if (fs->mountPoints().isEmpty()) {
            //We need to mount this disk first
            fs->mount()->then([ = ] {
                emit navigate(QUrl::fromLocalFile(fs->mountPoints().first()));
            })->error([ = ](QString error) {
                tDebug("Sidebar") << "Could not mount" << disk->displayName() << "-" << error;
            });
        } else {
            emit navigate(QUrl::fromLocalFile(fs->mountPoints().first()));
        }
    } else if (encrypted) {
        UnlockEncryptedPopover* jp = new UnlockEncryptedPopover(disk);
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &UnlockEncryptedPopover::reject, popover, &tPopover::dismiss);
        connect(jp, &UnlockEncryptedPopover::accept, this, [ = ](DiskObject * cleartext) {
            popover->dismiss();
            mount(cleartext);
        });
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &UnlockEncryptedPopover::deleteLater);
        popover->show(this->window());
    }
}
