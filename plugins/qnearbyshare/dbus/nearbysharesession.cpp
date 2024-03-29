#include "nearbysharesession.h"

#include <QCoroDBusPendingCall>
#include <QDBusInterface>
#include <QDBusMetaType>

struct NearbyShareSessionPrivate {
        QDBusInterface* session;

        QString peerName;
        bool isSending;
        QString pin;
        QString state;
        QString failedReason;
};

QDBusArgument& operator<<(QDBusArgument& argument, const NearbyShareSession::TransferProgress& transferProgress) {
    argument.beginStructure();
    argument << transferProgress.fileName << transferProgress.destination << transferProgress.transferred << transferProgress.size << transferProgress.complete;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, NearbyShareSession::TransferProgress& transferProgress) {
    argument.beginStructure();
    argument >> transferProgress.fileName >> transferProgress.destination >> transferProgress.transferred >> transferProgress.size >> transferProgress.complete;
    argument.endStructure();
    return argument;
}

NearbyShareSession::~NearbyShareSession() {
    delete d;
}

QString NearbyShareSession::peerName() {
    return d->peerName;
}

bool NearbyShareSession::isSending() {
    return d->isSending;
}

QString NearbyShareSession::pin() {
    return d->pin;
}

QString NearbyShareSession::state() {
    return d->state;
}

QString NearbyShareSession::failedReason() {
    return d->failedReason;
}

QCoro::Task<QList<NearbyShareSession::TransferProgress>> NearbyShareSession::transfers() {
    auto transfersMessage = co_await d->session->asyncCall("Transfers");
    QList<NearbyShareSession::TransferProgress> transfers;
    auto transfersArg = transfersMessage.arguments().first().value<QDBusArgument>();
    transfersArg >> transfers;
    co_return transfers;
}

void NearbyShareSession::accept() {
    d->session->asyncCall("AcceptTransfer");
}

void NearbyShareSession::reject() {
    d->session->asyncCall("RejectTransfer");
}

void NearbyShareSession::sessionPropertiesChanged(QString interface, QVariantMap properties, QStringList changedProperties) {
    if (properties.contains("State")) {
        d->state = properties.value("State").toString();
        d->failedReason = d->session->property("FailedReason").toString();
        d->pin = d->session->property("Pin").toString();
        emit stateChanged();
    }
}

NearbyShareSession::NearbyShareSession(QString path, QObject* parent) :
    QObject{parent} {
    qDBusRegisterMetaType<TransferProgress>();
    qDBusRegisterMetaType<QList<TransferProgress>>();

    d = new NearbyShareSessionPrivate();
    d->session = new QDBusInterface("com.vicr123.qnearbyshare", path, "com.vicr123.qnearbyshare.Session", QDBusConnection::sessionBus(), this);

    QDBusConnection::sessionBus().connect("com.vicr123.qnearbyshare", path, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(sessionPropertiesChanged(QString, QVariantMap, QStringList)));
    QDBusConnection::sessionBus().connect("com.vicr123.qnearbyshare", path, "com.vicr123.qnearbyshare.Session", "TransfersChanged", this, SIGNAL(transfersChanged(QList<NearbyShareSession::TransferProgress>)));

    d->peerName = d->session->property("PeerName").toString();
    d->isSending = d->session->property("IsSending").toBool();
    d->pin = d->session->property("Pin").toString();
    d->state = d->session->property("State").toString();
    d->failedReason = d->session->property("FailedReason").toString();
}
