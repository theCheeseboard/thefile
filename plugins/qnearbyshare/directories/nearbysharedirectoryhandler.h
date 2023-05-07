#ifndef NEARBYSHAREDIRECTORYHANDLER_H
#define NEARBYSHAREDIRECTORYHANDLER_H

#include <directoryhandler.h>

class NearbyShareDirectoryHandler : public DirectoryHandler {
        Q_OBJECT
    public:
        explicit NearbyShareDirectoryHandler(QObject* parent = nullptr);

    signals:

        // DirectoryHandler interface
    public:
        DirectoryPtr directoryForUrl(QUrl url);
        DirectoryPtr parentDirectoryForUrl(QUrl url);
        QString relativePath(QUrl from, QUrl to);
};

#endif // NEARBYSHAREDIRECTORYHANDLER_H
