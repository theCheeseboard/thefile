#include "placessidebarsection.h"

#include "../sidebarsection.h"
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QMenu>
#include <QMimeData>
#include <QStandardPaths>
#include <QUrl>
#include <resourcemanager.h>
#include <tlogger.h>

struct PlacesSidebarSectionPrivate {
        QListWidget* list;
};

PlacesSidebarSection::PlacesSidebarSection(QObject* parent) :
    SidebarSection{parent} {
    d = new PlacesSidebarSectionPrivate();

    d->list = new QListWidget();
    connect(d->list, &QListWidget::destroyed, this, &PlacesSidebarSection::deleteLater);

    struct Place {
            QString name;
            QIcon icon;
            QUrl location;
    };

    for (const Place& place : QList<Place>({
             {tr("Home"),         QIcon::fromTheme("go-home"),          QUrl::fromLocalFile(QDir::homePath())                                                   },
             {tr("Documents"),    QIcon::fromTheme("folder-documents"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))},
             {tr("Downloads"),    QIcon::fromTheme("folder-downloads"), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)) },
             {tr("Music"),        QIcon::fromTheme("folder-music"),     QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MusicLocation))    },
             {tr("Pictures"),     QIcon::fromTheme("folder-pictures"),  QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)) },
             {tr("Videos"),       QIcon::fromTheme("folder-videos"),    QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))   },
             {tr("Root"),         QIcon::fromTheme("folder-root"),      QUrl::fromLocalFile(QDir::rootPath())                                                   },
             {tr("Trash"),        QIcon::fromTheme("user-trash"),       QUrl("trash:/")                                                                         },
             {tr("Nearby Share"), QIcon::fromTheme("nearby-share"),     QUrl("nearbyshare:")                                                                    }
    })) {
        if (ResourceManager::directoryForUrl(place.location)) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(place.name);
            item->setIcon(place.icon);
            item->setData(Qt::UserRole, place.location);
            d->list->addItem(item);
        }
    }

    d->list->setFixedHeight(d->list->sizeHintForRow(0) * d->list->count());

    d->list->setAcceptDrops(true);
    d->list->installEventFilter(this);

    connect(d->list, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        emit navigate(item->data(Qt::UserRole).toUrl());
    });
}

PlacesSidebarSection::~PlacesSidebarSection() {
    delete d;
}

QString PlacesSidebarSection::label() {
    return tr("Places");
}

QWidget* PlacesSidebarSection::widget() {
    return d->list;
}

bool PlacesSidebarSection::eventFilter(QObject* watched, QEvent* event) {
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched);
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
        e->acceptProposedAction();
        return true;
    } else if (event->type() == QEvent::Drop) {
        QDropEvent* e = static_cast<QDropEvent*>(event);

        const QMimeData* mimeData = e->mimeData();
        tDebug("FileColumn") << mimeData->formats();
        QModelIndex index = view->indexAt(e->position().toPoint());

        if (mimeData->hasUrls()) {
            QList<QUrl> urls = mimeData->urls();
            if (index.isValid()) {
                QUrl url = index.data(Qt::UserRole).toUrl();
                DirectoryPtr dir = ResourceManager::directoryForUrl(url);
                if (url.scheme() == "trash") {
                    // Trash these items
                    for (const QUrl& url : qAsConst(urls)) {
                        ResourceManager::parentDirectoryForUrl(url)->trash(url.fileName());
                    }
                } else if (dir && QCoro::waitFor(dir->exists())) {
                    QMenu* menu = new QMenu();
                    menu->addSection(tr("For %1").arg(QLocale().quoteString(menu->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, SC_DPI_W(300, view)))));
                    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy In"), this, [this, urls, dir] {
                        emit copyFiles(urls, dir);
                    });
                    menu->addAction(QIcon::fromTheme("edit-cut"), tr("Move In"), this, [this, urls, dir] {
                        emit moveFiles(urls, dir);
                    });
                    menu->popup(view->mapToGlobal(e->position().toPoint()));
                    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
                }
            }
        }
        e->setDropAction(Qt::CopyAction);
        return true;
    }
    return false;
}
