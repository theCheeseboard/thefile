#include "recoveryidevicerootdirectory.h"

#include "recoveryidevice.h"
#include "recoveryidevicewidget.h"

struct RecoveryIDeviceRootDirectoryPrivate {
        QPointer<RecoveryIDevice> device;
};

RecoveryIDeviceRootDirectory::RecoveryIDeviceRootDirectory(RecoveryIDevice* device, QObject* parent) :
    Directory{parent} {
    d = new RecoveryIDeviceRootDirectoryPrivate();
    d->device = device;
}

RecoveryIDeviceRootDirectory::~RecoveryIDeviceRootDirectory() {
    delete d;
}

QCoro::Task<bool> RecoveryIDeviceRootDirectory::exists() {
    co_return true;
}

bool RecoveryIDeviceRootDirectory::isFile(QString path) {
    return false;
}

QUrl RecoveryIDeviceRootDirectory::url() {
    return QUrl(QStringLiteral("ios://e%1/").arg(d->device->ecid()));
}

quint64 RecoveryIDeviceRootDirectory::listCount(QDir::Filters filters, QDir::SortFlags sortFlags) {
    return 0;
}

QCoro::Generator<Directory::FileInformation> RecoveryIDeviceRootDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset) {
    co_return;
}

QCoro::Task<Directory::FileInformation> RecoveryIDeviceRootDirectory::fileInformation(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<QIODevice*> RecoveryIDeviceRootDirectory::open(QString filename, QIODeviceBase::OpenMode mode) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> RecoveryIDeviceRootDirectory::mkpath(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool RecoveryIDeviceRootDirectory::canTrash(QString filename) {
    return false;
}

QCoro::Task<QUrl> RecoveryIDeviceRootDirectory::trash(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> RecoveryIDeviceRootDirectory::deleteFile(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool RecoveryIDeviceRootDirectory::canMove(QString filename, QUrl to) {
    return false;
}

QCoro::Task<> RecoveryIDeviceRootDirectory::move(QString filename, QUrl to) {
    throw DirectoryOperationException("Not implemented");
}

QVariant RecoveryIDeviceRootDirectory::special(QString operation, QVariantMap args) {
    return {};
}

FileColumnWidget* RecoveryIDeviceRootDirectory::renderedWidget() {
    return new RecoveryIDeviceWidget(d->device);
}

Directory::ViewType RecoveryIDeviceRootDirectory::viewType() {
    return Directory::ViewType::Wide;
}

QString RecoveryIDeviceRootDirectory::columnTitle() {
    return d->device->deviceName();
}
