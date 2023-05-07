#include "nearbysharelistening.h"

#include <QDBusInterface>

struct NearbyShareListeningPrivate {
        QDBusInterface* listening;
};

NearbyShareListening::~NearbyShareListening() {
    this->stopListening();
    delete d;
}

void NearbyShareListening::stopListening() {
    d->listening->asyncCall("StopListening");
}

NearbyShareListening::NearbyShareListening(QDBusObjectPath path, QObject* parent) :
    QObject{parent} {
    d = new NearbyShareListeningPrivate();
    d->listening = new QDBusInterface("com.vicr123.qnearbyshare", path.path(), "com.vicr123.qnearbyshare.Listener", QDBusConnection::sessionBus(), this);
}
