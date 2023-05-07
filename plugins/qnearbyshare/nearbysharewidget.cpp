#include "nearbysharewidget.h"
#include "ui_nearbysharewidget.h"

#include "dbus/nearbysharelistening.h"
#include "dbus/nearbysharemanager.h"
#include "dbus/nearbysharetargetdiscovery.h"

#include "nearbysharetarget.h"

struct NearbyShareWidgetPrivate {
        NearbyShareManager manager;
        NearbyShareTargetDiscovery* targetDiscovery = nullptr;
        NearbyShareListening* listening = nullptr;

        QList<NearbyShareTarget*> targets;
};

NearbyShareWidget::NearbyShareWidget(QWidget* parent) :
    FileColumnWidget(parent),
    ui(new Ui::NearbyShareWidget) {
    ui->setupUi(this);

    d = new NearbyShareWidgetPrivate();
    ui->leftWidget->setFixedWidth(300);

    ui->discoverableLabel->setText(tr("Temporarily discoverable as %1.").arg(QLocale().quoteString(d->manager.serverName())));

    connect(&d->manager, &NearbyShareManager::newSessionAvailable, this, [this](NearbyShareSessionPtr session) {
        for (auto target : qAsConst(d->targets)) {
            if (target->name() == session->peerName()) {
                target->trackSession(session);
                return;
            }
        }

        auto targetWidget = new NearbyShareTarget(this);
        targetWidget->setName(session->peerName());
        //                targetWidget->setDevice(static_cast<NearbyShareTarget::Device>(target.deviceType));
        targetWidget->setDevice(NearbyShareTarget::Device::Unknown);
        //        targetWidget->setConnectionString(target.connectionString);
        d->targets.append(targetWidget);
        ui->targetsLayout->addWidget(targetWidget);

        targetWidget->trackSession(session);
    });

    this->start();
}

NearbyShareWidget::~NearbyShareWidget() {
    if (d->targetDiscovery) {
        d->targetDiscovery->deleteLater();
    }
    if (d->listening) {
        d->listening->deleteLater();
    }

    delete d;
    delete ui;
}

QCoro::Task<> NearbyShareWidget::start() {
    d->targetDiscovery = co_await d->manager.discoverTargets();
    if (d->targetDiscovery) {
        connect(d->targetDiscovery, &NearbyShareTargetDiscovery::discoveredNewTarget, this, [this](NearbyShareTargetDiscovery::NearbyShareTarget target) {
            // TODO: See if we can update a dead target first

            auto targetWidget = new NearbyShareTarget(this);
            targetWidget->setName(target.name);
            targetWidget->setDevice(static_cast<NearbyShareTarget::Device>(target.deviceType));
            targetWidget->setConnectionString(target.connectionString);
            d->targets.append(targetWidget);
            ui->targetsLayout->addWidget(targetWidget);
        });
        connect(d->targetDiscovery, &NearbyShareTargetDiscovery::discoveredTargetGone, this, [this](QString targetString) {
            // TODO: Only make targets disappear if they don't have any tracked sessions

            for (auto target : d->targets) {
                if (target->connectionString() == targetString) {
                    d->targets.removeAll(target);
                    ui->targetsLayout->removeWidget(target);
                    target->deleteLater();
                    return;
                }
            }
        });
    }

    d->listening = co_await d->manager.startListening();
}
