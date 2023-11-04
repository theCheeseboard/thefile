#ifndef ABSTRACTIDEVICE_H
#define ABSTRACTIDEVICE_H

#include <QObject>
#include <QString>

class AbstractIDevice : public QObject {
        Q_OBJECT

    public:
        AbstractIDevice(QObject* parent = nullptr);

        virtual QString deviceName() = 0;
        virtual QString productType() = 0;
        virtual QString deviceClass() = 0;
        virtual QIcon icon() = 0;
        virtual quint64 ecid() = 0;

        virtual QString humanReadableProductVersion(QString productVersion) = 0;
};

#endif // ABSTRACTIDEVICE_H
