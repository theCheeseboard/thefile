#ifndef ISOFTWAREUPDATEFILE_H
#define ISOFTWAREUPDATEFILE_H

#include <QObject>

struct ISoftwareUpdateFilePrivate;
class ISoftwareUpdateFile : public QObject {
        Q_OBJECT
    public:
        explicit ISoftwareUpdateFile(QString path, QObject* parent = nullptr);
        ~ISoftwareUpdateFile();

        bool isValid();

        QString productVersion();
        QString productBuildVersion();
        QStringList supportedProductTypes();

    signals:

    private:
        ISoftwareUpdateFilePrivate* d;
};

#endif // ISOFTWAREUPDATEFILE_H
