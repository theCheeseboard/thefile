#ifndef NEARBYSHARESESSION_H
#define NEARBYSHARESESSION_H

#include <QObject>

class NearbyShareManager;
struct NearbyShareSessionPrivate;
class NearbyShareSession : public QObject {
        Q_OBJECT
    public:
        ~NearbyShareSession();

        QString peerName();
        bool isSending();
        QString pin();
        QString state();
        QString failedReason();

        void accept();
        void reject();

    private slots:
        void sessionPropertiesChanged(QString interface, QVariantMap properties, QStringList changedProperties);

    signals:
        void stateChanged();

    protected:
        friend NearbyShareManager;
        explicit NearbyShareSession(QString path, QObject* parent = nullptr);

    private:
        NearbyShareSessionPrivate* d;
};

typedef QSharedPointer<NearbyShareSession> NearbyShareSessionPtr;

#endif // NEARBYSHARESESSION_H
