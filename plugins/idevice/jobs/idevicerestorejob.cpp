#include "idevicerestorejob.h"

#include "idevice.h"
#include "progress/idevicerestorejobprogress.h"
#include "tnotification.h"
#include <QCoroFuture>
#include <QCoroNetwork>
#include <QCryptographicHash>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <QtConcurrent>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <tlogger.h>
#include <unistd.h>

struct IDeviceRestoreJobPrivate {
        quint64 progress = 0;
        quint64 totalProgress = 0;
        QString description;
        tJob::State state = tJob::Processing;

        bool erase;
        QPointer<AbstractIDevice> device;
        QString deviceName;
        QString deviceClass;

        bool canCancel = true;
        std::stop_source stopSource;
        std::stop_token stopToken;

        bool haveStorageGroup() {
            // Get the current user
            uid_t uid = geteuid();
            struct passwd* pw = getpwuid(uid);
            if (!pw) {
                return false;
            }

            // Get the group
            struct group* grp = getgrnam("storage");
            if (!grp) {
                return false;
            }

            // Check if user is in the group
            for (char** p = grp->gr_mem; *p; p++) {
                if (strcmp(*p, pw->pw_name) == 0) {
                    return true;
                }
            }

            return false;
        }
};

IDeviceRestoreJob::IDeviceRestoreJob(bool erase, AbstractIDevice* device, QObject* parent) :
    tJob{parent} {
    d = new IDeviceRestoreJobPrivate();
    d->erase = erase;
    d->device = device;
    d->deviceName = device->deviceName();
    d->deviceClass = device->deviceClass();
    d->stopToken = d->stopSource.get_token();

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

QCoro::Task<> IDeviceRestoreJob::downloadAndStartRestore(QString buildId, QString softwareVersion, QString sha256) {
    d->description = tr("Preparing to download %1").arg(softwareVersion);
    emit descriptionChanged(d->description);

    auto softwareFile = co_await downloadSoftware(buildId, softwareVersion);
    while (!co_await checkSoftwareFile(softwareFile, sha256)) {
        QFile::remove(softwareFile);
        if (d->stopSource.stop_requested()) break;
        softwareFile = co_await downloadSoftware(buildId, softwareVersion);
    }

    if (d->stopSource.stop_requested()) {
        if (d->erase) {
            d->description = tr("Cancelled restore operation");
        } else {
            d->description = tr("Cancelled update operation");
        }
        emit descriptionChanged(d->description);

        d->state = tJob::Failed;
        emit stateChanged(d->state);

        d->totalProgress = 100;
        d->progress = 100;
        emit totalProgressChanged(d->totalProgress);
        emit progressChanged(d->progress);

        co_return;
    }

    startRestore(softwareFile, softwareVersion);
}

void IDeviceRestoreJob::startRestore(QString softwareFile, QString softwareVersion) {
    if (d->erase) {
        d->description = tr("Preparing for restore");
    } else {
        d->description = tr("Preparing for update");
    }
    emit descriptionChanged(d->description);

    d->totalProgress = 0;
    d->progress = 0;
    emit totalProgressChanged(d->totalProgress);
    emit progressChanged(d->progress);

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

                if (stage == 0 || stage == 1 || stage == 6) { // Warmup
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

    if (d->haveStorageGroup()) {
        // We don't need root access in order to recover devices
        restoreProcess->start(args.takeFirst(), args);
    } else {
        restoreProcess->start("pkexec", args);
    }
}

void IDeviceRestoreJob::cancel() {
    if (!d->canCancel) return;

    d->canCancel = false;
    emit canCancelChanged(d->canCancel);

    d->stopSource.request_stop();
}

QCoro::Task<QString> IDeviceRestoreJob::downloadSoftware(QString buildId, QString softwareVersion) {
    QDir cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir appleDir(cacheDir.absoluteFilePath("apple-restore"));
    QDir::root().mkpath(appleDir.absolutePath());

    auto softwareFile = appleDir.absoluteFilePath(buildId);
    if (QFile::exists(softwareFile)) co_return softwareFile;

    QUrl url(QStringLiteral("https://api.ipsw.me/v4/ipsw/download/%1/%2").arg(d->device->productType(), buildId));

    QFile file(softwareFile);
    file.open(QFile::WriteOnly);

    QNetworkAccessManager mgr;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "theFile/" + QApplication::applicationVersion());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);

    auto reply = mgr.get(req);
    connect(reply, &QNetworkReply::redirected, this, [reply](const QUrl& url) {
        if (url.host() == "appldnld.apple.com" || url.scheme() == "https") {
            reply->redirectAllowed();
        }
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [softwareVersion, this](qint64 bytesReceived, qint64 bytesTotal) {
        d->description = tr("Downloading %1\n%2 of %3").arg(softwareVersion, QLocale().formattedDataSize(bytesReceived), QLocale().formattedDataSize(bytesTotal));
        emit descriptionChanged(d->description);

        d->totalProgress = bytesTotal / 1024 / 1024;
        d->progress = bytesReceived / 1024 / 1024;
        emit totalProgressChanged(d->totalProgress);
        emit progressChanged(d->progress);
    });

    while (co_await qCoro(reply).waitForReadyRead(30000)) {
        if (d->stopSource.stop_requested()) break;
        file.write(reply->readAll());

        if (reply->isFinished()) break;
    }

    file.write(reply->readAll());

    file.close();

    if (!reply->isFinished() || reply->error() != QNetworkReply::NoError) {
        // TODO
        tWarn("IDeviceRestoreJob") << "Did not finish downloading " << softwareVersion << ": " << reply->errorString();
        co_return softwareFile;
    }

    co_return softwareFile;
}

QCoro::Task<bool> IDeviceRestoreJob::checkSoftwareFile(QString softwareFile, QString sha256) {
    d->description = tr("Checking the downloaded system software");
    emit descriptionChanged(d->description);

    d->totalProgress = 0;
    d->progress = 0;
    emit totalProgressChanged(d->totalProgress);
    emit progressChanged(d->progress);

    co_return co_await QtConcurrent::run([](QString softwareFile, QString sha256) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        QFile file(softwareFile);
        file.open(QFile::ReadOnly);
        hash.addData(&file);

        auto result = hash.result();
        auto hex = result.toHex();

        if (hex.toLower() != sha256.toLower()) {
            tWarn("IDeviceRestoreJob") << "Download integrity check failed";
            tWarn("IDeviceRestoreJob") << "Expected SHA256 Hash: " << sha256;
            tWarn("IDeviceRestoreJob") << "Actual SHA256 Hash:   " << QString(hex);
            return false;
        }

        return true;
    },
        softwareFile, sha256);
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
