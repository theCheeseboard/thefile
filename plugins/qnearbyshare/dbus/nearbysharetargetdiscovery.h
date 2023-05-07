#ifndef NEARBYSHARETARGETDISCOVERY_H
#define NEARBYSHARETARGETDISCOVERY_H

#include <QDBusObjectPath>
#include <QObject>

class NearbyShareManager;
struct NearbyShareTargetDiscoveryPrivate;
class NearbyShareTargetDiscovery : public QObject {
        Q_OBJECT
    public:
        ~NearbyShareTargetDiscovery();

        struct NearbyShareTarget {
                QString connectionString;
                QString name;
                uint deviceType;
        };

        void stopDiscovery();

    signals:
        void discoveredNewTarget(NearbyShareTargetDiscovery::NearbyShareTarget target);
        void discoveredTargetGone(QString connectionString);

    protected:
        friend NearbyShareManager;
        explicit NearbyShareTargetDiscovery(QDBusObjectPath path, QObject* parent = nullptr);

    private:
        NearbyShareTargetDiscoveryPrivate* d;
};

Q_DECLARE_METATYPE(NearbyShareTargetDiscovery::NearbyShareTarget)

#endif // NEARBYSHARETARGETDISCOVERY_H
