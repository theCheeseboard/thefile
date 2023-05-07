#include "nearbysharetarget.h"
#include "ui_nearbysharetarget.h"

#include "dbus/nearbysharemanager.h"
#include "nearbysharetargetsession.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <ticon.h>

struct NearbyShareTargetPrivate {
        NearbyShareManager manager;
        QString peerName;
        QString connectionString;
        bool sendable = false;
        bool haveTrackedSessions = false;
};

NearbyShareTarget::NearbyShareTarget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NearbyShareTarget) {
    ui->setupUi(this);
    d = new NearbyShareTargetPrivate();
    ui->topWidget->installEventFilter(this);
    ui->topWidget->setAcceptDrops(true);
}

NearbyShareTarget::~NearbyShareTarget() {
    delete ui;
    delete d;
}

void NearbyShareTarget::setName(QString peerName) {
    d->peerName = peerName;
    ui->nameLabel->setText(peerName);
}

void NearbyShareTarget::setConnectionString(QString connectionString) {
    d->connectionString = connectionString;
}

void NearbyShareTarget::setDevice(Device device) {
    switch (device) {
        case Device::Tablet:
            ui->iconLabel->setPixmap(QIcon::fromTheme("tablet").pixmap(QSize(32, 32)));
            break;
        case Device::Computer:
            ui->iconLabel->setPixmap(QIcon::fromTheme("computer").pixmap(QSize(32, 32)));
            break;
        case Device::Unknown:
        case Device::Phone:
        default:
            ui->iconLabel->setPixmap(QIcon::fromTheme("phone").pixmap(QSize(32, 32)));
            break;
    }
}

void NearbyShareTarget::setSendable(bool sendable) {
    d->sendable = sendable;
}

QString NearbyShareTarget::name() {
    return d->peerName;
}

QString NearbyShareTarget::connectionString() {
    return d->connectionString;
}

void NearbyShareTarget::trackSession(NearbyShareSessionPtr session) {
    auto ts = new NearbyShareTargetSession(session, this);
    ui->sessionsLayout->addWidget(ts);
    d->haveTrackedSessions = true;
}

bool NearbyShareTarget::haveTrackedSessions() {
    return d->haveTrackedSessions;
}

bool NearbyShareTarget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->topWidget) {
        if (event->type() == QEvent::DragEnter) {
            auto e = static_cast<QDragEnterEvent*>(event);
            e->acceptProposedAction();
            return true;
        } else if (event->type() == QEvent::Drop) {
            auto e = static_cast<QDropEvent*>(event);
            const QMimeData* mimeData = e->mimeData();

            if (!mimeData->hasUrls()) return false;

            QList<QUrl> urls = mimeData->urls();
            d->manager.send(d->connectionString, d->peerName, urls);
            e->setDropAction(Qt::CopyAction);

            return true;
        }
    }
    return false;
}
