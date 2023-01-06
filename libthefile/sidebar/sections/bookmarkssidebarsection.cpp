#include "bookmarkssidebarsection.h"

#include "../bookmarksmodel.h"
#include <QListView>
#include <QMenu>
#include <QTimer>
#include <bookmarkmanager.h>
#include <tapplication.h>

struct BookmarksSidebarSectionPrivate {
        QListView* list;
        BookmarksModel* bookmarksModel;
};

BookmarksSidebarSection::BookmarksSidebarSection(QObject* parent) :
    SidebarSection{parent} {
    d = new BookmarksSidebarSectionPrivate();

    d->bookmarksModel = new BookmarksModel();
    d->list = new QListView();
    connect(d->bookmarksModel, &BookmarksModel::modelReset, this, [this] {
        d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->bookmarksModel->rowCount());
        this->setVisibility(d->bookmarksModel->rowCount() != 0);
    });
    QTimer::singleShot(0, [this] {
        d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->bookmarksModel->rowCount());
    });
    d->list->setModel(d->bookmarksModel);
    d->list->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setVisibility(d->bookmarksModel->rowCount() != 0);

    connect(d->list, &QListView::activated, this, [this](QModelIndex index) {
        // Ignore if we're trying to right click
        if (tApplication::mouseButtons() & Qt::RightButton) return;

        // Navigate to the item
        emit navigate(index.data(BookmarksModel::UrlRole).toUrl());
    });
    connect(d->list, &QListView::customContextMenuRequested, this, [this](QPoint pos) {
        QModelIndex bookmark = d->list->indexAt(pos);

        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(bookmark.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI_W(300, d->list)))));
        menu->addAction(QIcon::fromTheme("bookmark-remove"), tr("Remove from bookmarks"), this, [=] {
            BookmarkManager::instance()->removeBookmark(bookmark.data(BookmarksModel::UrlRole).toUrl());
        });
        menu->popup(d->list->mapToGlobal(pos));
    });
}

BookmarksSidebarSection::~BookmarksSidebarSection() {
    delete d;
}

QString BookmarksSidebarSection::label() {
    return tr("Bookmarks");
}

QWidget* BookmarksSidebarSection::widget() {
    return d->list;
}
