#include "idevicesidebarsection.h"

#include "idevicemodel.h"
#include "popovers/idevicerestorepopover.h"
#include <QListView>
#include <QMenu>
#include <QTimer>
#include <abstractidevice.h>
#include <tapplication.h>
#include <tpopover.h>

struct IDeviceSidebarSectionPrivate {
        IDeviceWatcher* watcher;
        QListView* list;
        IDeviceModel* model;
};

IDeviceSidebarSection::IDeviceSidebarSection(IDeviceWatcher* watcher, QObject* parent) :
    SidebarSection{parent} {
    d = new IDeviceSidebarSectionPrivate();
    d->watcher = watcher;

    d->list = new QListView();
    d->model = new IDeviceModel(watcher, this);
    connect(d->model, &IDeviceModel::modelReset, this, [this] {
        d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->model->rowCount());
        this->setVisibility(d->model->rowCount() != 0);
    });
    QTimer::singleShot(0, [this] {
        d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->model->rowCount());
    });
    d->list->setModel(d->model);
    d->list->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setVisibility(d->model->rowCount() != 0);

    connect(d->list, &QListView::activated, this, [this](QModelIndex index) {
        // Ignore if we're trying to right click
        if (tApplication::mouseButtons() & Qt::RightButton) return;

        // Navigate to the item
        emit navigate(index.data(IDeviceModel::UrlRole).toUrl());
    });
    connect(d->list, &QListView::customContextMenuRequested, this, [this](QPoint pos) {
        QModelIndex index = d->list->indexAt(pos);
        auto device = index.data(IDeviceModel::DeviceRole).value<AbstractIDevice*>();

        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI_W(300, d->list)))));
        menu->addAction(QIcon::fromTheme("phone-upgrade"), tr("Update System Software"), this, [this, device] {
            IDeviceRestorePopover* jp = new IDeviceRestorePopover(device, false);
            tPopover* popover = new tPopover(jp);
            popover->setPopoverWidth(SC_DPI(-200));
            popover->setPopoverSide(tPopover::Bottom);
            connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
            popover->show(d->list->window());
        });
        menu->addAction(QIcon::fromTheme("phone-erase"), tr("Restore System Software"), this, [this, device] {
            IDeviceRestorePopover* jp = new IDeviceRestorePopover(device, true);
            tPopover* popover = new tPopover(jp);
            popover->setPopoverWidth(SC_DPI(-200));
            popover->setPopoverSide(tPopover::Bottom);
            connect(jp, &IDeviceRestorePopover::done, popover, &tPopover::dismiss);
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, jp, &IDeviceRestorePopover::deleteLater);
            popover->show(d->list->window());
        });
        menu->popup(d->list->mapToGlobal(pos));
    });
}

IDeviceSidebarSection::~IDeviceSidebarSection() {
    delete d;
}

QString IDeviceSidebarSection::label() {
    return tr("Apple");
}

QWidget* IDeviceSidebarSection::widget() {
    return d->list;
}
