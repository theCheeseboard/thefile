#ifndef IDEVICEDIRECTORYHANDLER_H
#define IDEVICEDIRECTORYHANDLER_H

#include <directoryhandler.h>

class IDeviceWatcher;
struct IDeviceDirectoryHandlerPrivate;
class IDeviceDirectoryHandler : public DirectoryHandler {
        Q_OBJECT
    public:
        explicit IDeviceDirectoryHandler(IDeviceWatcher* watcher, QObject* parent = nullptr);
        ~IDeviceDirectoryHandler();

    signals:

    private:
        IDeviceDirectoryHandlerPrivate* d;

        // DirectoryHandler interface
    public:
        DirectoryPtr directoryForUrl(QUrl url);
        DirectoryPtr parentDirectoryForUrl(QUrl url);
        QString relativePath(QUrl from, QUrl to);
};

#endif // IDEVICEDIRECTORYHANDLER_H
