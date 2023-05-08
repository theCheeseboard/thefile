#include "nearbysharemanager.h"

#include "nearbysharelistening.h"
#include "nearbysharesession.h"
#include "nearbysharetargetdiscovery.h"
#include <QCoroDBusPendingCall>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QUrl>

struct NearbyShareManagerPrivate {
        QString serverName;
        QDBusInterface* managerInterface;
        QMap<QString, NearbyShareSessionPtr> sessions;
};

QDBusArgument& operator<<(QDBusArgument& argument, const NearbyShareManager::SendingFile& file) {
    argument.beginStructure();
    argument << file.fd << file.filename;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, NearbyShareManager::SendingFile& file) {
    argument.beginStructure();
    argument >> file.fd >> file.filename;
    argument.endStructure();
    return argument;
}

NearbyShareManager::NearbyShareManager(QObject* parent) :
    QObject{parent} {
    qDBusRegisterMetaType<SendingFile>();
    qDBusRegisterMetaType<QList<SendingFile>>();

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

QCoro::Task<NearbyShareSessionPtr> NearbyShareManager::send(QString connectionString, QString peerName, QList<QFile*> files) {
    QList<SendingFile> sendingFiles;
    for (auto file : files) {
        QFileInfo fileInfo(file->fileName());
        sendingFiles.append({QDBusUnixFileDescriptor(file->handle()),
            fileInfo.fileName()});
    }

    auto reply = co_await d->managerInterface->asyncCall("SendToTarget", connectionString, peerName, QVariant::fromValue(sendingFiles));
    if (reply.type() != QDBusMessage::ReplyMessage) {
        co_return {};
    }

    auto sessionPath = reply.arguments().first().value<QDBusObjectPath>();
    co_return session(sessionPath.path());
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

QCoro::Task<NearbyShareSessionPtr> NearbyShareManager::send(QString connectionString, QString peerName, QList<QUrl> files) {
    QList<QFile*> fileList;
    for (auto url : files) {
        if (url.isLocalFile()) {
            auto file = new QFile(url.toLocalFile());
            file->open(QFile::ReadOnly);
            fileList.append(file);
        }
    }

    if (fileList.isEmpty()) co_return {};

    auto retval = co_await this->send(connectionString, peerName, fileList);
    for (auto file : fileList) {
        file->close();
        file->deleteLater();
    }
    co_return retval;
}
