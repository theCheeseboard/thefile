#include "nearbysharewidget.h"
#include "ui_nearbysharewidget.h"

#include <tpopover.h>

#include "dbus/nearbysharelistening.h"
#include "dbus/nearbysharemanager.h"
#include "dbus/nearbysharetargetdiscovery.h"

#include "nearbysharehelppopover.h"
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

    connect(&d->manager, &NearbyShareManager::newSessionAvailable, this, &NearbyShareWidget::addNewSession);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

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
            for (auto targetWidget : d->targets) {
                if (targetWidget->connectionString() == target.connectionString) {
                    targetWidget->setName(target.name);
                    targetWidget->setDevice(static_cast<NearbyShareTarget::Device>(target.deviceType));
                    targetWidget->setSendable(true);
                    return;
                }
            }

            for (auto targetWidget : d->targets) {
                if (targetWidget->connectionString().isEmpty() && targetWidget->name() == target.name) {
                    targetWidget->setConnectionString(target.connectionString);
                    targetWidget->setDevice(static_cast<NearbyShareTarget::Device>(target.deviceType));
                    targetWidget->setSendable(true);
                    return;
                }
            }

            auto targetWidget = new NearbyShareTarget(this);
            targetWidget->setName(target.name);
            targetWidget->setDevice(static_cast<NearbyShareTarget::Device>(target.deviceType));
            targetWidget->setConnectionString(target.connectionString);
            targetWidget->setSendable(true);
            d->targets.append(targetWidget);
            ui->targetsLayout->addWidget(targetWidget);
            ui->stackedWidget->setCurrentWidget(ui->targetsPage);
        });
        connect(d->targetDiscovery, &NearbyShareTargetDiscovery::discoveredTargetGone, this, [this](QString targetString) {
            for (auto target : d->targets) {
                if (target->connectionString() == targetString) {
                    if (target->haveTrackedSessions()) {
                        target->setSendable(false);
                        return;
                    }

                    d->targets.removeAll(target);
                    ui->targetsLayout->removeWidget(target);
                    target->deleteLater();

                    if (d->targets.isEmpty()) ui->stackedWidget->setCurrentWidget(ui->noTargetsPage);
                    return;
                }
            }
        });
    }

    d->listening = co_await d->manager.startListening();

    for (auto session : co_await d->manager.sessions()) {
        if (session->state() != QStringLiteral("Complete") && session->state() != QStringLiteral("Failed")) {
            addNewSession(session);
        }
    }
}

void NearbyShareWidget::addNewSession(NearbyShareSessionPtr session) {
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
    ui->stackedWidget->setCurrentWidget(ui->targetsPage);

    targetWidget->trackSession(session);
}

void NearbyShareWidget::on_helpButton_clicked() {
    auto jp = new NearbyShareHelpPopover();
    auto popover = new tPopover(jp);
    popover->setPopoverWidth(-200);
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &NearbyShareHelpPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &NearbyShareHelpPopover::deleteLater);
    popover->show(this->window());
}
