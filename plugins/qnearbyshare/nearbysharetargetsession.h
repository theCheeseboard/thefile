#ifndef NEARBYSHARETARGETSESSION_H
#define NEARBYSHARETARGETSESSION_H

#include "dbus/nearbysharesession.h"
#include <QWidget>

namespace Ui {
    class NearbyShareTargetSession;
}

struct NearbyShareTargetSessionPrivate;
class NearbyShareTargetSession : public QWidget {
        Q_OBJECT

    public:
        explicit NearbyShareTargetSession(NearbyShareSessionPtr session, QWidget* parent = nullptr);
        ~NearbyShareTargetSession();

    private slots:
        void on_acceptButton_clicked();

        void on_declineButton_clicked();

    private:
        Ui::NearbyShareTargetSession* ui;
        NearbyShareTargetSessionPrivate* d;

        void updateState();
        void updateDetails();
        void updateTransfers(QList<NearbyShareSession::TransferProgress> transfers);
};

#endif // NEARBYSHARETARGETSESSION_H
