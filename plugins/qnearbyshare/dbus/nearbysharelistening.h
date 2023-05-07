#ifndef NEARBYSHARELISTENING_H
#define NEARBYSHARELISTENING_H

#include <QDBusObjectPath>
#include <QObject>

class NearbyShareManager;
struct NearbyShareListeningPrivate;
class NearbyShareListening : public QObject {
        Q_OBJECT
    public:
        ~NearbyShareListening();

        void stopListening();

    signals:

    protected:
        friend NearbyShareManager;
        explicit NearbyShareListening(QDBusObjectPath path, QObject* parent = nullptr);

    private:
        NearbyShareListeningPrivate* d;
};

#endif // NEARBYSHARELISTENING_H
