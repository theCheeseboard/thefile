#ifndef NEARBYSHAREMANAGER_H
#define NEARBYSHAREMANAGER_H

#include <QCoroTask>
#include <QDBusObjectPath>
#include <QObject>

#include "nearbysharesession.h"

class NearbyShareListening;
class NearbyShareTargetDiscovery;
struct NearbyShareManagerPrivate;
class NearbyShareManager : public QObject {
        Q_OBJECT
    public:
        explicit NearbyShareManager(QObject* parent = nullptr);
        ~NearbyShareManager();

        QString serverName();

        QCoro::Task<NearbyShareListening*> startListening();
        QCoro::Task<NearbyShareTargetDiscovery*> discoverTargets();
        QCoro::Task<QList<NearbyShareSessionPtr>> sessions();

    private slots:
        void newSession(QDBusObjectPath sessionPath);

    signals:
        void newSessionAvailable(NearbyShareSessionPtr session);

    private:
        NearbyShareManagerPrivate* d;

        NearbyShareSessionPtr session(QString path);
};

#endif // NEARBYSHAREMANAGER_H
