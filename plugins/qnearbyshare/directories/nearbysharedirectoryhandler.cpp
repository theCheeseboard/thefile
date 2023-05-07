#include "nearbysharedirectoryhandler.h"

#include "nearbysharerootdirectory.h"

NearbyShareDirectoryHandler::NearbyShareDirectoryHandler(QObject* parent) :
    DirectoryHandler{parent} {
}

DirectoryPtr NearbyShareDirectoryHandler::directoryForUrl(QUrl url) {
    if (url.scheme() != "nearbyshare") return nullptr;

    if (url.path() == "") return (new NearbyShareRootDirectory())->sharedFromThis();
    return nullptr;
}

DirectoryPtr NearbyShareDirectoryHandler::parentDirectoryForUrl(QUrl url) {
    return nullptr;
}

QString NearbyShareDirectoryHandler::relativePath(QUrl from, QUrl to) {
    return "";
}
