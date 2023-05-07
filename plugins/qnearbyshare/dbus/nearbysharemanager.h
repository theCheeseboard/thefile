#ifndef NEARBYSHAREMANAGER_H
#define NEARBYSHAREMANAGER_H

#include <QObject>

struct NearbyShareManagerPrivate;
class NearbyShareManager : public QObject {
        Q_OBJECT
    public:
        explicit NearbyShareManager(QObject* parent = nullptr);
        ~NearbyShareManager();

        QString serverName();

    signals:

    private:
        NearbyShareManagerPrivate* d;
};

#endif // NEARBYSHAREMANAGER_H
