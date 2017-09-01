#include "drivesmodel.h"

extern QString calculateSize(quint64 size);

DrivesModel::DrivesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(reloadUdisks()));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(reloadUdisks()));
    reloadUdisks();

    qDBusRegisterMetaType<ObjectManagerDBus>();
}

int DrivesModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return items.count();
}

QVariant DrivesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return items.at(index.row()).text();
            break;
        case Qt::DecorationRole:
            return items.at(index.row()).icon();
            break;
        case Qt::UserRole:
            return QVariant::fromValue(items.at(index.row()));
            break;
    }

    return QVariant();
}


QDBusArgument &operator<<(QDBusArgument &argument, const ObjectManagerDBus &objectManager)
{
    argument.beginStructure();
    argument << objectManager.path << objectManager.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ObjectManagerDBus &objectManager)
{
    argument.beginStructure();
    argument >> objectManager.path >> objectManager.properties;
    argument.endStructure();
    return argument;
}

void DrivesModel::reloadUdisks() {
    items.clear();
    items.append(PaneItem("/", "Root", QIcon::fromTheme("computer")));
    items.append(PaneItem(QDir::homePath(), "Home", QIcon::fromTheme("go-home")));

    QDBusMessage getDrivesMessage = QDBusMessage::createMethodCall("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    QDBusMessage objects = QDBusConnection::systemBus().call(getDrivesMessage);
    QDBusArgument objectArg = objects.arguments().first().value<QDBusArgument>();

    QList<ObjectManagerDBus> availableObjects;
    objectArg >> availableObjects;

    for (ObjectManagerDBus object : availableObjects) {
        if (object.path.path().startsWith("/org/freedesktop/UDisks2/block_devices")) {
            PaneItem drive(object.path);

            if (drive.itemType() != PaneItem::Invalid) {
                items.append(drive);
            }
        }
    }

    emit dataChanged(index(0), index(rowCount()));
}

PaneItem::PaneItem() {
    type = Invalid;
}

PaneItem::PaneItem(QDBusObjectPath path) {
    this->path = path.path();

    interface = new QDBusInterface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    fsInterface = new QDBusInterface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
    driveInterface = new QDBusInterface("org.freedesktop.UDisks2", interface->property("Drive").value<QDBusObjectPath>().path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

    QString usage = interface->property("IdUsage").toString();
    if (usage != "filesystem") {
        type = Invalid;
        return;
    } else {
        if (interface->property("HintIgnore").toBool()) {
            type = Invalid;
            return;
        } else {
            QString label = interface->property("IdLabel").toString();
            if (label == "") {
                qulonglong size = interface->property("Size").toULongLong();

                tx = calculateSize(size) + " Disk";
            } else {
                tx = label;
            }

            QIcon ic;
            if (driveInterface->isValid()) {
                if (driveInterface->property("Removable").toBool()) {
                    ic = QIcon::fromTheme("drive-removable-media");
                } else {
                    ic = QIcon::fromTheme("drive-harddisk");
                }
            } else {
                ic = QIcon::fromTheme("package");
            }
            this->ic = ic;
        }
    }

    type = Disk;
}

PaneItem::PaneItem(QString path, QString name, QIcon icon) {
    this->path = path;
    this->tx = name;
    this->ic = icon;
    type = Filesystem;
}

PaneItem::~PaneItem() {
    if (this->type == Disk) {
        //if (interface != NULL) interface->deleteLater();
        //if (driveInterface != NULL) driveInterface->deleteLater();
    }
}

PaneItem::PaneItemType PaneItem::itemType() {
    return type;
}

QString PaneItem::text() const {
    return tx;
}

QIcon PaneItem::icon() const {
    return ic;
}

QStringList PaneItem::mountPoints() const {
    if (this->type == Disk) {
        QDBusMessage getMountPoints = QDBusMessage::createMethodCall(fsInterface->service(), fsInterface->path(), "org.freedesktop.DBus.Properties", "Get");
        getMountPoints.setArguments(QList<QVariant>() << "org.freedesktop.UDisks2.Filesystem" << "MountPoints");
        QDBusArgument mountPointsUnmarshalled = QDBusConnection::systemBus().call(getMountPoints).arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
        QList<QString> mountPoints;

        mountPointsUnmarshalled.beginArray();
        while (!mountPointsUnmarshalled.atEnd()) {
            QByteArray mountPoint;
            mountPointsUnmarshalled >> mountPoint;

            QString mountPointString = mountPoint;
            mountPoints.append(mountPointString);
        }
        mountPointsUnmarshalled.endArray();

        return mountPoints;
    } else {
        return QStringList();
    }
}

QString PaneItem::attemptMount() const {
    QDBusMessage reply = fsInterface->call("Mount", QVariantMap());
    if (reply.errorName() == "") {
        return reply.arguments().first().toString();
    } else {
        return reply.errorMessage();
    }
}

QString PaneItem::attemptUnmount() const {
    QDBusMessage reply = fsInterface->call("Unmount", QVariantMap());
    if (reply.errorName() == "") {
        return "";
    } else {
        QString error = reply.errorName();
        if (error == "org.freedesktop.UDisks2.Error.DeviceBusy") {
            return "The disk is currently being used.";
        } else {
            return reply.errorMessage();
        }
    }
}

QString PaneItem::forceUnmount() const {
    QVariantMap options;
    options.insert("force", true);

    QDBusMessage reply = fsInterface->call("Unmount", options);
    if (reply.errorName() == "") {
        return "";
    } else {
        QString error = reply.errorName();
        if (error == "org.freedesktop.UDisks2.Error.DeviceBusy") {
            return "The disk is currently being used.";
        } else {
            return reply.errorMessage();
        }
    }
}

QString PaneItem::filePath() {
    if (this->type == Filesystem) {
        return path;
    }
    return "";
}

bool PaneItem::isMounted() {
    if (this->type == Disk) {
        if (mountPoints().count() == 0) {
            return false;
        } else {
            return true;
        }
    } else {
        return true;
    }
}

void DrivesDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex &index) const {
    QStyleOptionViewItem newOption = option;
    newOption.rect.adjust(3, 0, 0, 0);
    QStyledItemDelegate::paint(painter, newOption, index);

    PaneItem item = index.data(Qt::UserRole).value<PaneItem>();
    if (item.isMounted() && item.itemType() == PaneItem::Disk) {
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect.left(), option.rect.top(), 3, option.rect.height());
    }
}

QSize DrivesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}
