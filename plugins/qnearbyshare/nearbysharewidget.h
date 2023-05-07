#ifndef NEARBYSHAREWIDGET_H
#define NEARBYSHAREWIDGET_H

#include <QCoroTask>
#include <filecolumnwidget.h>

#include "dbus/nearbysharesession.h"

namespace Ui {
    class NearbyShareWidget;
}

struct NearbyShareWidgetPrivate;
class NearbyShareWidget : public FileColumnWidget {
        Q_OBJECT

    public:
        explicit NearbyShareWidget(QWidget* parent = nullptr);
        ~NearbyShareWidget();

    private:
        Ui::NearbyShareWidget* ui;
        NearbyShareWidgetPrivate* d;

        QCoro::Task<> start();
        void addNewSession(NearbyShareSessionPtr session);
};

#endif // NEARBYSHAREWIDGET_H
