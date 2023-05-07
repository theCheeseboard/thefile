#ifndef NEARBYSHARETARGET_H
#define NEARBYSHARETARGET_H

#include "dbus/nearbysharesession.h"
#include <QWidget>

namespace Ui {
    class NearbyShareTarget;
}

struct NearbyShareTargetPrivate;
class NearbyShareTarget : public QWidget {
        Q_OBJECT

    public:
        explicit NearbyShareTarget(QWidget* parent = nullptr);
        ~NearbyShareTarget();

        enum class Device : uint {
            Unknown = 0,
            Phone = 1,
            Tablet = 2,
            Computer = 3
        };

        void setName(QString peerName);
        void setConnectionString(QString connectionString);
        void setDevice(Device device);
        void setSendable(bool sendable);

        QString name();
        QString connectionString();
        void trackSession(NearbyShareSessionPtr session);
        bool haveTrackedSessions();

    private:
        Ui::NearbyShareTarget* ui;
        NearbyShareTargetPrivate* d;
};

#endif // NEARBYSHARETARGET_H
