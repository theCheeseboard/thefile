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

    private slots:
        void on_helpButton_clicked();

        void on_label_5_linkActivated(const QString& link);

        void on_helpLabel_linkActivated(const QString& link);

    private:
        Ui::NearbyShareWidget* ui;
        NearbyShareWidgetPrivate* d;

        QCoro::Task<> start();
        void addNewSession(NearbyShareSessionPtr session);

        void showHelp();
};

#endif // NEARBYSHAREWIDGET_H
