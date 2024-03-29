#include "nearbysharetargetsession.h"
#include "ui_nearbysharetargetsession.h"

#include <QProgressBar>

struct NearbyShareTargetSessionPrivate {
        NearbyShareSessionPtr session;
        QList<QProgressBar*> progressBars;
};

NearbyShareTargetSession::NearbyShareTargetSession(NearbyShareSessionPtr session, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NearbyShareTargetSession) {
    ui->setupUi(this);

    d = new NearbyShareTargetSessionPrivate();
    d->session = session;

    connect(d->session.data(), &NearbyShareSession::stateChanged, this, &NearbyShareTargetSession::updateState);
    connect(d->session.data(), &NearbyShareSession::transfersChanged, this, &NearbyShareTargetSession::updateTransfers);
    updateState();

    d->session->transfers().then([this](QList<NearbyShareSession::TransferProgress> transfers) {
        this->updateTransfers(transfers);
    });

    updateDetails();
}

NearbyShareTargetSession::~NearbyShareTargetSession() {
    delete ui;
    delete d;
}

void NearbyShareTargetSession::updateState() {
    auto state = d->session->state();
    if (state == QStringLiteral("NotReady")) {
        ui->statusLabel->setText(tr("Preparing..."));
        ui->statusButtons->setVisible(false);
    } else if (state == QStringLiteral("WaitingForUserAccept")) {
        updateDetails();

        if (d->session->isSending()) {
            ui->statusLabel->setText(tr("Waiting for the peer to accept the connection..."));
            ui->statusButtons->setVisible(false);
        } else {
            ui->statusLabel->setText(tr("Accept this transfer?"));
            ui->statusButtons->setVisible(true);
        }
    } else if (state == QStringLiteral("Transferring")) {
        if (d->session->isSending()) {
            ui->statusLabel->setText(tr("Sending files..."));
        } else {
            ui->statusLabel->setText(tr("Receiving files..."));
        }
        ui->statusButtons->setVisible(false);
    } else if (state == QStringLiteral("Complete")) {
        ui->statusLabel->setText(tr("Transfer complete."));
        ui->statusButtons->setVisible(false);
    } else if (state == QStringLiteral("Failed")) {
        auto failedReason = d->session->failedReason();
        if (failedReason == QStringLiteral("RemoteDeclined")) {
            ui->statusLabel->setText(tr("The peer declined the transfer."));
        } else if (failedReason == QStringLiteral("RemoteOutOfSpace")) {
            ui->statusLabel->setText(tr("The peer does not have sufficient disk space to complete the transfer."));
        } else if (failedReason == QStringLiteral("RemoteTimedOut")) {
            ui->statusLabel->setText(tr("The transfer timed out."));
        } else {
            ui->statusLabel->setText(tr("The transfer failed."));
        }
        ui->statusButtons->setVisible(false);
    }
}

void NearbyShareTargetSession::updateDetails() {
    ui->pinLabel->setText(tr("PIN: %1").arg(d->session->pin()));
}

void NearbyShareTargetSession::updateTransfers(QList<NearbyShareSession::TransferProgress> transfers) {
    for (auto i = 0; i < transfers.length(); i++) {
        auto transfer = transfers.at(i);

        if (d->progressBars.count() <= i) {
            auto label = new QLabel(this);
            label->setText(transfer.fileName);
            ui->transfersLayout->addWidget(label, i, 0);

            auto progress = new QProgressBar(this);
            ui->transfersLayout->addWidget(progress, i, 1);
            d->progressBars.append(progress);
        }

        auto progress = d->progressBars.at(i);
        progress->setMaximum(transfer.size);
        progress->setValue(transfer.transferred);
    }
}

void NearbyShareTargetSession::on_acceptButton_clicked() {
    d->session->accept();
}

void NearbyShareTargetSession::on_declineButton_clicked() {
    d->session->reject();
}
