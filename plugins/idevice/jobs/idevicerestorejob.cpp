#include "idevicerestorejob.h"

#include "idevice.h"
#include "progress/idevicerestorejobprogress.h"
#include "tnotification.h"
#include <QPointer>
#include <QProcess>
#include <tlogger.h>

struct IDeviceRestoreJobPrivate {
        quint64 progress = 0;
        quint64 totalProgress = 0;
        QString description;
        tJob::State state = tJob::Processing;

        bool erase;
        QPointer<IDevice> device;
        QString deviceName;
        QString deviceClass;

        bool canCancel = true;
        bool cancelled = false;
};

IDeviceRestoreJob::IDeviceRestoreJob(bool erase, IDevice* device, QObject* parent) :
    tJob{parent} {
    d = new IDeviceRestoreJobPrivate();
    d->erase = erase;
    d->device = device;
    d->deviceName = device->deviceName();
    d->deviceClass = device->deviceClass();

    connect(this, &IDeviceRestoreJob::descriptionChanged, this, &IDeviceRestoreJob::statusStringChanged);

    d->description = tr("Waiting for download to complete");
}

IDeviceRestoreJob::~IDeviceRestoreJob() {
    delete d;
}

QString IDeviceRestoreJob::description() {
    return d->description;
}

bool IDeviceRestoreJob::isErase() {
    return d->erase;
}

bool IDeviceRestoreJob::canCancel() {
    return d->canCancel;
}

QString IDeviceRestoreJob::deviceName() {
    return d->deviceName;
}

void IDeviceRestoreJob::startRestore(QString softwareFile, QString softwareVersion) {
    if (d->erase) {
        d->description = tr("Preparing for restore");
    } else {
        d->description = tr("Preparing for update");
    }
    emit descriptionChanged(d->description);

    d->canCancel = false;
    emit canCancelChanged(d->canCancel);

    QProcess* restoreProcess = new QProcess();
    QStringList args = {"idevicerestore",
        "--ecid", QString::number(d->device->ecid()), "--no-input", "--plain-progress", softwareFile};
    if (d->erase) args.append("--erase");

    connect(restoreProcess, &QProcess::readyReadStandardOutput, this, [this, restoreProcess] {
        while (restoreProcess->canReadLine()) {
            QString line = restoreProcess->readLine().trimmed();
            tDebug("IDeviceRestoreJob") << line;

            if (line.startsWith("progress:")) {
                auto parts = line.split(" ");
                auto stage = parts.at(1).toInt();
                auto progress = parts.at(2).toDouble();

                if (stage == 1) { // Warmup
                    d->totalProgress = 0;
                    d->progress = 0;
                    if (d->erase) {
                        d->description = tr("Preparing for restore");
                    } else {
                        d->description = tr("Preparing for update");
                    }
                } else if (stage == 2) { // Update
                    if (d->erase) {
                        d->description = tr("Restoring %1").arg(d->deviceClass);
                    } else {
                        d->description = tr("Updating %1").arg(d->deviceClass);
                    }
                    d->totalProgress = 2000;
                    d->progress = progress * 1000;
                } else if (stage == 3) { // Verify
                    if (d->erase) {
                        d->description = tr("Verifying restored software");
                    } else {
                        d->description = tr("Verifying updated software");
                    }
                    d->totalProgress = 2000;
                    d->progress = progress * 1000 + 1000;
                } else {
                    d->description = tr("Finishing up");
                    d->totalProgress = 0;
                    d->progress = 0;
                }

                emit descriptionChanged(d->description);
                emit totalProgressChanged(d->totalProgress);
                emit progressChanged(d->progress);
            }
        }
    });
    connect(restoreProcess, &QProcess::finished, this, [this, softwareVersion](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
            d->state = tJob::Finished;
            emit stateChanged(d->state);

            if (d->erase) {
                d->description = tr("Restore complete");
            } else {
                d->description = tr("Update complete");
            }
            emit descriptionChanged(d->description);

            tNotification* notification = new tNotification();
            if (d->erase) {
                notification->setSummary(tr("Restored system software to %1").arg(QLocale().quoteString(d->deviceName)));
            } else {
                notification->setSummary(tr("Updated system software on %1").arg(QLocale().quoteString(d->deviceName)));
            }
            notification->setText(tr("%1 was installed onto %2.").arg(softwareVersion, QLocale().quoteString(d->deviceName)));
            notification->post();
        } else {
            d->state = tJob::Failed;
            emit stateChanged(d->state);

            if (d->erase) {
                d->description = tr("The restore operation failed");
            } else {
                d->description = tr("The update operation failed");
            }
            emit descriptionChanged(d->description);

            tNotification* notification = new tNotification();
            if (d->erase) {
                notification->setSummary(tr("Couldn't restore to %1").arg(QLocale().quoteString(d->deviceName)));
                notification->setText(tr("Could not install %1 on %2.").arg(QLocale().quoteString(d->deviceName), softwareVersion));
            } else {
                notification->setSummary(tr("Couldn't update %1").arg(QLocale().quoteString(d->deviceName)));
                notification->setText(tr("Could not update %1 to %2.").arg(QLocale().quoteString(d->deviceName), softwareVersion));
            }
            notification->post();
        }
    });

    restoreProcess->start("pkexec", args);
}

void IDeviceRestoreJob::cancel() {
    if (!d->canCancel) return;
}

quint64 IDeviceRestoreJob::progress() {
    return d->progress;
}

quint64 IDeviceRestoreJob::totalProgress() {
    return d->totalProgress;
}

IDeviceRestoreJob::State IDeviceRestoreJob::state() {
    return d->state;
}

QWidget* IDeviceRestoreJob::makeProgressWidget() {
    return new IDeviceRestoreJobProgress(this);
}

QString IDeviceRestoreJob::titleString() {
    if (this->isErase()) {
        return tr("Restore %1").arg(QLocale().quoteString(this->deviceName()));
    } else {
        return tr("Update %1").arg(QLocale().quoteString(this->deviceName()));
    }
}

QString IDeviceRestoreJob::statusString() {
    return this->description();
}
