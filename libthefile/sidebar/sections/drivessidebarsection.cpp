#include "drivessidebarsection.h"

#include "../devicesmodel.h"
#include "popovers/unlockencryptedpopover.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/encryptedinterface.h>
#include <DriveObjects/filesysteminterface.h>
#include <QListView>
#include <QMenu>
#include <diskoperationmanager.h>
#include <frisbeeexception.h>
#include <tlogger.h>
#include <tmessagebox.h>
#include <tpopover.h>

struct DrivesSidebarSectionPrivate {
        QListView* list;
        DevicesModel* devicesModel;
};

DrivesSidebarSection::DrivesSidebarSection(QObject* parent) :
    SidebarSection{parent} {
    d = new DrivesSidebarSectionPrivate();
    d->list = new QListView();

    d->devicesModel = new DevicesModel();
    d->list->setModel(d->devicesModel);
    connect(d->devicesModel, &DevicesModel::modelReset, this, [this] {
        d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->devicesModel->rowCount());
    });
    d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->devicesModel->rowCount());
    d->list->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(d->list, &QListView::activated, this, [this](QModelIndex index) {
        // Ignore if we're trying to right click
        if (qApp->mouseButtons() & Qt::RightButton) return;

        // Mount and navigate to the item
        DiskObject* disk = index.data(DevicesModel::DiskObjectRole).value<DiskObject*>();
        mount(disk);
    });
    connect(d->list, &QListView::customContextMenuRequested, this, [this](QPoint pos) {
        QModelIndex device = d->list->indexAt(pos);
        DiskObject* disk = device.data(DevicesModel::DiskObjectRole).value<DiskObject*>();
        FilesystemInterface* fs = disk->interface<FilesystemInterface>();
        BlockInterface* block = disk->interface<BlockInterface>();
        DriveInterface* drive = nullptr;
        if (block) drive = block->drive();

        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(device.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI(300)))));
        if (fs) {
            if (fs->mountPoints().isEmpty()) {
                menu->addAction(QIcon::fromTheme("media-mount"), tr("Mount"), this, [=] {
                    fs->mount();
                });
            } else {
                menu->addAction(QIcon::fromTheme("media-unmount"), tr("Unmount"), this, [=]() -> QCoro::Task<> {
                    try {
                        co_await fs->unmount();
                    } catch (FrisbeeException& ex) {
                        tMessageBox* box = new tMessageBox(d->list);
                        box->setTitleBarText(tr("Couldn't unmount"));
                        box->setMessageText(tr("Unmounting the drive failed."));
                        box->setInformativeText(ex.response());
                        box->show(true);
                    }
                });
            }
        }

        if (drive) {
            if (drive->ejectable()) menu->addAction(QIcon::fromTheme("media-eject"), tr("Eject"), this, [=]() -> QCoro::Task<> {
                try {
                    co_await drive->eject();
                } catch (FrisbeeException& ex) {
                    tMessageBox* box = new tMessageBox(d->list);
                    box->setTitleBarText(tr("Couldn't eject"));
                    box->setMessageText(tr("Ejecting the drive failed."));
                    box->setInformativeText(ex.response());
                    box->show(true);
                }
            });
        }

        if (block && block->cryptoBackingDevice()) {
            menu->addAction(tr("Lock"), this, [=]() -> QCoro::Task<> {
                auto performLock = [=]() -> QCoro::Task<> {
                    try {
                        co_await block->cryptoBackingDevice()->interface<EncryptedInterface>()->lock();
                    } catch (FrisbeeException& ex) {
                        tMessageBox* box = new tMessageBox(d->list);
                        box->setTitleBarText(tr("Couldn't lock"));
                        box->setMessageText(tr("Locking the device failed."));
                        box->setInformativeText(ex.response());
                        box->show(true);
                    }
                };

                // Unmount the drive first, and then lock it
                if (fs->mountPoints().isEmpty()) {
                    co_await performLock();
                } else {
                    try {
                        co_await fs->unmount();
                        co_await performLock();
                    } catch (FrisbeeException& ex) {
                        tMessageBox* box = new tMessageBox(d->list);
                        box->setTitleBarText(tr("Couldn't unmount"));
                        box->setMessageText(tr("Unmounting the drive failed."));
                        box->setInformativeText(ex.response());
                        box->show(true);
                    }
                }
            });
        }

        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("media-image-create"), tr("Create Disk Image"), this, [=] {
            DiskOperationManager::showDiskOperationUi(d->list, DiskOperationManager::Image, disk);
        });

        QString eraseText = tr("Erase");
        QIcon eraseIcon = QIcon::fromTheme("media-harddisk-erase");
        if (drive && drive->isOpticalDrive()) {
            eraseText = tr("Erase Optical Disc");
            eraseIcon = QIcon::fromTheme("media-optical-erase");
        }

        menu->addAction(eraseIcon, eraseText, this, [=] {
            DiskOperationManager::showDiskOperationUi(d->list, DiskOperationManager::Erase, disk);
        });

        menu->popup(d->list->mapToGlobal(pos));
    });
}

DrivesSidebarSection::~DrivesSidebarSection() {
    delete d;
}

QCoro::Task<> DrivesSidebarSection::mount(DiskObject* disk) {
    // Mount and navigate to the item
    FilesystemInterface* fs = disk->interface<FilesystemInterface>();
    EncryptedInterface* encrypted = disk->interface<EncryptedInterface>();
    if (fs) {
        if (fs->mountPoints().isEmpty()) {
            // We need to mount this disk first
            try {
                co_await fs->mount();
                emit navigate(QUrl::fromLocalFile(fs->mountPoints().first()));
            } catch (FrisbeeException& ex) {
                tDebug("Sidebar") << "Could not mount" << disk->displayName() << "-" << ex.response();
            }
        } else {
            emit navigate(QUrl::fromLocalFile(fs->mountPoints().first()));
        }
    } else if (encrypted) {
        UnlockEncryptedPopover* jp = new UnlockEncryptedPopover(disk);
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI_W(-200, d->list));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &UnlockEncryptedPopover::reject, popover, &tPopover::dismiss);
        connect(jp, &UnlockEncryptedPopover::accept, this, [=](DiskObject* cleartext) {
            popover->dismiss();
            mount(cleartext);
        });
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &UnlockEncryptedPopover::deleteLater);
        popover->show(d->list->window());
    }
}

QString DrivesSidebarSection::label() {
    return tr("Drives");
}

QWidget* DrivesSidebarSection::widget() {
    return d->list;
}
