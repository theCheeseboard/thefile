#include "idevicedirectoryhandler.h"

#include "idevicerootdirectory.h"
#include "idevicewatcher.h"
#include "recoveryidevicerootdirectory.h"

struct IDeviceDirectoryHandlerPrivate {
        IDeviceWatcher* watcher;
};

IDeviceDirectoryHandler::IDeviceDirectoryHandler(IDeviceWatcher* watcher, QObject* parent) :
    DirectoryHandler{parent} {
    d = new IDeviceDirectoryHandlerPrivate();
    d->watcher = watcher;
}

IDeviceDirectoryHandler::~IDeviceDirectoryHandler() {
    delete d;
}

DirectoryPtr IDeviceDirectoryHandler::directoryForUrl(QUrl url) {
    if (url.scheme() != "ios") return nullptr;

    auto host = url.host();
    if (host.startsWith("u")) {
        auto device = d->watcher->deviceByUdid(url.host().mid(1));
        if (!device) return nullptr;

        if (url.path() == "/") return (new IDeviceRootDirectory(device))->sharedFromThis();
        return nullptr;
    } else {
        auto device = d->watcher->recoveryDeviceByEcid(host.mid(1).toULongLong());
        if (!device) return nullptr;

        return (new RecoveryIDeviceRootDirectory(device))->sharedFromThis();
    }
}

DirectoryPtr IDeviceDirectoryHandler::parentDirectoryForUrl(QUrl url) {
    if (url.scheme() != "ios") return nullptr;
    return nullptr;
}

QString IDeviceDirectoryHandler::relativePath(QUrl from, QUrl to) {
    return "";
}
