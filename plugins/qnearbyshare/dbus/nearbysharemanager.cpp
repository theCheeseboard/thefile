#include "nearbysharemanager.h"

#include <QDBusInterface>

struct NearbyShareManagerPrivate {
        QString serverName;
        QDBusInterface* managerInterface;
};

NearbyShareManager::NearbyShareManager(QObject* parent) :
    QObject{parent} {
    d = new NearbyShareManagerPrivate();
    d->managerInterface = new QDBusInterface("com.vicr123.qnearbyshare", "/com/vicr123/qnearbyshare", "com.vicr123.qnearbyshare.Manager", QDBusConnection::sessionBus(), this);

    d->serverName = d->managerInterface->property("ServerName").toString();
}

NearbyShareManager::~NearbyShareManager() {
    delete d;
}

QString NearbyShareManager::serverName() {
    return d->serverName;
}
