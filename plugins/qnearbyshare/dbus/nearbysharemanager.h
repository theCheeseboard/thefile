#ifndef NEARBYSHAREMANAGER_H
#define NEARBYSHAREMANAGER_H

#include <QCoroTask>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QObject>

#include "nearbysharesession.h"

class QFile;
class NearbyShareListening;
class NearbyShareTargetDiscovery;
struct NearbyShareManagerPrivate;
class NearbyShareManager : public QObject {
        Q_OBJECT
    public:
        explicit NearbyShareManager(QObject* parent = nullptr);
        ~NearbyShareManager();

        struct SendingFile {
                QDBusUnixFileDescriptor fd;
                QString filename;
        };

        QString serverName();

        QCoro::Task<NearbyShareListening*> startListening();
        QCoro::Task<NearbyShareTargetDiscovery*> discoverTargets();
        QCoro::Task<QList<NearbyShareSessionPtr>> sessions();

        QCoro::Task<NearbyShareSessionPtr> send(QString connectionString, QString peerName, QList<QFile*> files);
        QCoro::Task<NearbyShareSessionPtr> send(QString connectionString, QString peerName, QList<QUrl> files);

    private slots:
        void newSession(QDBusObjectPath sessionPath);

    signals:
        void newSessionAvailable(NearbyShareSessionPtr session);

    private:
        NearbyShareManagerPrivate* d;

        NearbyShareSessionPtr session(QString path);
};

Q_DECLARE_METATYPE(NearbyShareManager::SendingFile)

#endif // NEARBYSHAREMANAGER_H
