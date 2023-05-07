#ifndef NEARBYSHAREWIDGET_H
#define NEARBYSHAREWIDGET_H

#include <QCoroTask>
#include <filecolumnwidget.h>

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
};

#endif // NEARBYSHAREWIDGET_H
