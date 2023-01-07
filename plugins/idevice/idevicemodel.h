#ifndef IDEVICEMODEL_H
#define IDEVICEMODEL_H

#include <QAbstractListModel>

class IDeviceWatcher;
struct IDeviceModelPrivate;
class IDeviceModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit IDeviceModel(IDeviceWatcher* watcher, QObject* parent = nullptr);
        ~IDeviceModel();

        enum Roles {
            DeviceRole = Qt::UserRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        IDeviceModelPrivate* d;
};

#endif // IDEVICEMODEL_H
