#include "nearbysharerootdirectory.h"

#include "nearbysharewidget.h"

NearbyShareRootDirectory::NearbyShareRootDirectory(QObject* parent) :
    Directory{parent} {
}

QCoro::Task<bool> NearbyShareRootDirectory::exists() {
    co_return true;
}

bool NearbyShareRootDirectory::isFile(QString path) {
    return false;
}

QUrl NearbyShareRootDirectory::url() {
    return QUrl("nearbyshare://");
}

quint64 NearbyShareRootDirectory::listCount(QDir::Filters filters, QDir::SortFlags sortFlags) {
    return 0;
}

QCoro::Generator<Directory::FileInformation> NearbyShareRootDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset) {
    co_return;
}

QCoro::Task<Directory::FileInformation> NearbyShareRootDirectory::fileInformation(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<QIODevice*> NearbyShareRootDirectory::open(QString filename, QIODeviceBase::OpenMode mode) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> NearbyShareRootDirectory::mkpath(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool NearbyShareRootDirectory::canTrash(QString filename) {
    return false;
}

QCoro::Task<QUrl> NearbyShareRootDirectory::trash(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> NearbyShareRootDirectory::deleteFile(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool NearbyShareRootDirectory::canMove(QString filename, QUrl to) {
    return false;
}

QCoro::Task<> NearbyShareRootDirectory::move(QString filename, QUrl to) {
    throw DirectoryOperationException("Not implemented");
}

QVariant NearbyShareRootDirectory::special(QString operation, QVariantMap args) {
    return {};
}

FileColumnWidget* NearbyShareRootDirectory::renderedWidget() {
    return new NearbyShareWidget();
}

Directory::ViewType NearbyShareRootDirectory::viewType() {
    return Directory::ViewType::Wide;
}

QString NearbyShareRootDirectory::columnTitle() {
    return tr("Nearby Share");
}
