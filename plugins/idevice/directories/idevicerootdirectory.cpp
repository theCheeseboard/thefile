#include "idevicerootdirectory.h"

#include "idevice.h"
#include "idevicefilecolumnwidget.h"

struct IDeviceRootDirectoryPrivate {
        QPointer<IDevice> device;
        QUrl url;
};

IDeviceRootDirectory::IDeviceRootDirectory(IDevice* device, QObject* parent) :
    Directory{parent} {
    d = new IDeviceRootDirectoryPrivate();
    d->device = device;

    d->url.setScheme("ios");
    d->url.setHost(QStringLiteral("u%1").arg(d->device->udid()));
    d->url.setPath("/");

    connect(device, &IDevice::destroyed, this, &IDeviceRootDirectory::contentsChanged);
}

IDeviceRootDirectory::~IDeviceRootDirectory() {
    delete d;
}

QCoro::Task<bool> IDeviceRootDirectory::exists() {
    // The root directory must always exist
    co_return true;
}

bool IDeviceRootDirectory::isFile(QString path) {
    return false;
}

QUrl IDeviceRootDirectory::url() {
    return d->url;
}

quint64 IDeviceRootDirectory::listCount(QDir::Filters filters, QDir::SortFlags sortFlags) {
    if (!d->device) return 0;
    return 1;
}

QCoro::Generator<Directory::FileInformation> IDeviceRootDirectory::list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset) {
    if (!d->device) co_return;

    FileInformation pictures;
    pictures.icon = QIcon::fromTheme("folder-pictures");
    pictures.name = tr("Pictures");
    pictures.filenameForFileOperations = "pictures";
    co_yield pictures;
}

QCoro::Task<Directory::FileInformation> IDeviceRootDirectory::fileInformation(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<QIODevice*> IDeviceRootDirectory::open(QString filename, QIODeviceBase::OpenMode mode) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> IDeviceRootDirectory::mkpath(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool IDeviceRootDirectory::canTrash(QString filename) {
    return false;
}

QCoro::Task<QUrl> IDeviceRootDirectory::trash(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

QCoro::Task<> IDeviceRootDirectory::deleteFile(QString filename) {
    throw DirectoryOperationException("Not implemented");
}

bool IDeviceRootDirectory::canMove(QString filename, QUrl to) {
    return false;
}

QCoro::Task<> IDeviceRootDirectory::move(QString filename, QUrl to) {
    throw DirectoryOperationException("Not implemented");
}

QVariant IDeviceRootDirectory::special(QString operation, QVariantMap args) {
    return QVariant();
}

QList<FileColumnWidget*> IDeviceRootDirectory::actions() {
    return {new IDeviceFileColumnWidget(d->device)};
}
