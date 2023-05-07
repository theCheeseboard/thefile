#include "nearbysharewidget.h"
#include "ui_nearbysharewidget.h"

#include <dbus/nearbysharemanager.h>

struct NearbyShareWidgetPrivate {
        NearbyShareManager manager;
};

NearbyShareWidget::NearbyShareWidget(QWidget* parent) :
    FileColumnWidget(parent),
    ui(new Ui::NearbyShareWidget) {
    ui->setupUi(this);

    d = new NearbyShareWidgetPrivate();
    ui->leftWidget->setFixedWidth(300);

    ui->discoverableLabel->setText(tr("Temporarily discoverable as %1.").arg(QLocale().quoteString(d->manager.serverName())));
}

NearbyShareWidget::~NearbyShareWidget() {
    delete d;
    delete ui;
}
