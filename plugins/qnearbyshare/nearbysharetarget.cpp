#include "nearbysharetarget.h"
#include "ui_nearbysharetarget.h"

#include "nearbysharetargetsession.h"
#include <ticon.h>

struct NearbyShareTargetPrivate {
        QString connectionString;
        bool sendable = false;
        bool haveTrackedSessions = false;
};

NearbyShareTarget::NearbyShareTarget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NearbyShareTarget) {
    ui->setupUi(this);
    d = new NearbyShareTargetPrivate();
}

NearbyShareTarget::~NearbyShareTarget() {
    delete ui;
    delete d;
}

void NearbyShareTarget::setName(QString peerName) {
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
    return ui->nameLabel->text();
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
