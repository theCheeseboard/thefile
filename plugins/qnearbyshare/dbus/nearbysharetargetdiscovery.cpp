#include "nearbysharetargetdiscovery.h"

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMetaType>

struct NearbyShareTargetDiscoveryPrivate {
        QDBusInterface* targetDiscovery;
};

QDBusArgument& operator<<(QDBusArgument& argument, const NearbyShareTargetDiscovery::NearbyShareTarget& target) {
    argument.beginStructure();
    argument << target.connectionString << target.name << target.deviceType;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, NearbyShareTargetDiscovery::NearbyShareTarget& target) {
    argument.beginStructure();
    argument >> target.connectionString >> target.name >> target.deviceType;
    argument.endStructure();
    return argument;
}

NearbyShareTargetDiscovery::~NearbyShareTargetDiscovery() {
    this->stopDiscovery();
    delete d;
}

void NearbyShareTargetDiscovery::stopDiscovery() {
    d->targetDiscovery->asyncCall("StopDiscovery");
}

NearbyShareTargetDiscovery::NearbyShareTargetDiscovery(QDBusObjectPath path, QObject* parent) :
    QObject{parent} {
    qDBusRegisterMetaType<NearbyShareTarget>();
    qDBusRegisterMetaType<QList<NearbyShareTarget>>();

    d = new NearbyShareTargetDiscoveryPrivate();
    d->targetDiscovery = new QDBusInterface("com.vicr123.qnearbyshare", path.path(), "com.vicr123.qnearbyshare.TargetDiscovery", QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect("com.vicr123.qnearbyshare", path.path(), "com.vicr123.qnearbyshare.TargetDiscovery", "DiscoveredNewTarget", this, SIGNAL(discoveredNewTarget(NearbyShareTargetDiscovery::NearbyShareTarget)));
    QDBusConnection::sessionBus().connect("com.vicr123.qnearbyshare", path.path(), "com.vicr123.qnearbyshare.TargetDiscovery", "DiscoveredTargetGone", this, SIGNAL(discoveredTargetGone(QString)));
}
