#include "nearbysharemanager.h"

#include "nearbysharelistening.h"
#include "nearbysharesession.h"
#include "nearbysharetargetdiscovery.h"
#include <QCoroDBusPendingCall>
#include <QDBusArgument>
#include <QDBusInterface>

struct NearbyShareManagerPrivate {
        QString serverName;
        QDBusInterface* managerInterface;
        QMap<QString, NearbyShareSessionPtr> sessions;
};

NearbyShareManager::NearbyShareManager(QObject* parent) :
    QObject{parent} {
    d = new NearbyShareManagerPrivate();
    d->managerInterface = new QDBusInterface("com.vicr123.qnearbyshare", "/com/vicr123/qnearbyshare", "com.vicr123.qnearbyshare.Manager", QDBusConnection::sessionBus(), this);

    d->serverName = d->managerInterface->property("ServerName").toString();

    QDBusConnection::sessionBus().connect("com.vicr123.qnearbyshare", "/com/vicr123/qnearbyshare", "com.vicr123.qnearbyshare.Manager", "NewSession", this, SLOT(newSession(QDBusObjectPath)));
}

NearbyShareManager::~NearbyShareManager() {
    delete d;
}

QString NearbyShareManager::serverName() {
    return d->serverName;
}

QCoro::Task<NearbyShareListening*> NearbyShareManager::startListening() {
    auto reply = co_await d->managerInterface->asyncCall("StartListening");
    if (reply.type() != QDBusMessage::ReplyMessage) {
        co_return nullptr;
    }

    auto path = reply.arguments().constFirst().value<QDBusObjectPath>();
    co_return new NearbyShareListening(path);
}

QCoro::Task<NearbyShareTargetDiscovery*> NearbyShareManager::discoverTargets() {
    auto reply = co_await d->managerInterface->asyncCall("DiscoverTargets");
    if (reply.type() != QDBusMessage::ReplyMessage) {
        co_return nullptr;
    }

    auto path = reply.arguments().constFirst().value<QDBusObjectPath>();
    co_return new NearbyShareTargetDiscovery(path);
}

QCoro::Task<QList<NearbyShareSessionPtr>> NearbyShareManager::sessions() {
    auto reply = co_await d->managerInterface->asyncCall("Sessions");
    if (reply.type() != QDBusMessage::ReplyMessage) {
        co_return {};
    }

    QList<NearbyShareSessionPtr> sessions;
    auto pathsArg = reply.arguments().constFirst().value<QDBusArgument>();
    QList<QDBusObjectPath> paths;
    pathsArg >> paths;
    for (auto path : paths) {
        sessions.append(this->session(path.path()));
    }
    co_return sessions;
}

void NearbyShareManager::newSession(QDBusObjectPath sessionPath) {
    emit newSessionAvailable(session(sessionPath.path()));
}

NearbyShareSessionPtr NearbyShareManager::session(QString path) {
    if (d->sessions.contains(path)) {
        return d->sessions.value(path);
    }

    auto session = NearbyShareSessionPtr(new NearbyShareSession(path));
    d->sessions.insert(path, session);
    return session;
}
