#ifndef DRIVESMODEL_H
#define DRIVESMODEL_H

#include <QAbstractListModel>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QIcon>
#include <QDir>
#include <QStyledItemDelegate>
#include <QPainter>

class PaneItem
{
public:
    PaneItem();
    PaneItem(QDBusObjectPath path);
    PaneItem(QString path, QString name, QIcon icon);
    ~PaneItem();

    enum PaneItemType {
        Invalid,
        Disk,
        Filesystem
    };

    PaneItemType itemType();
    QString text() const;
    QIcon icon() const;
    QStringList mountPoints();
    QString attemptMount();
    QString filePath();
    bool isMounted();

private:
    QDBusInterface *interface, *driveInterface, *fsInterface;
    QString path;
    QString tx;
    QIcon ic;
    PaneItemType type = Invalid;
};
Q_DECLARE_METATYPE(PaneItem)

struct ObjectManagerDBus
{
    QDBusObjectPath path;
    QDBusVariant properties;
};
Q_DECLARE_METATYPE(ObjectManagerDBus)

class DrivesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DrivesModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void reloadUdisks();

private:
    QList<PaneItem> items;
};

class DrivesDelegate : public QStyledItemDelegate
{

public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DRIVESMODEL_H
