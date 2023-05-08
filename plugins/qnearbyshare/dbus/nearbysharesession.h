#ifndef NEARBYSHARESESSION_H
#define NEARBYSHARESESSION_H

#include <QCoroTask>
#include <QObject>

class NearbyShareManager;
struct NearbyShareSessionPrivate;
class NearbyShareSession : public QObject {
        Q_OBJECT
    public:
        ~NearbyShareSession();

        struct TransferProgress {
                QString fileName;
                QString destination;
                quint64 size;

                quint64 transferred = 0;
                bool complete = false;
        };

        QString peerName();
        bool isSending();
        QString pin();
        QString state();
        QString failedReason();

        QCoro::Task<QList<NearbyShareSession::TransferProgress>> transfers();

        void accept();
        void reject();

    private slots:
        void sessionPropertiesChanged(QString interface, QVariantMap properties, QStringList changedProperties);

    signals:
        void stateChanged();
        void transfersChanged(QList<NearbyShareSession::TransferProgress> transfers);

    protected:
        friend NearbyShareManager;
        explicit NearbyShareSession(QString path, QObject* parent = nullptr);

    private:
        NearbyShareSessionPrivate* d;
};
Q_DECLARE_METATYPE(NearbyShareSession::TransferProgress)

typedef QSharedPointer<NearbyShareSession> NearbyShareSessionPtr;

#endif // NEARBYSHARESESSION_H
