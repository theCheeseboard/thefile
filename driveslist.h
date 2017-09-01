#ifndef DRIVESLIST_H
#define DRIVESLIST_H

#include <QWidget>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <ttoast.h>
#include <QProcess>
#include "drivesmodel.h"
#include "forceunmountdialog.h"

class DrivesList : public QListView
{
    Q_OBJECT
public:
    explicit DrivesList(QWidget *parent = nullptr);

signals:
    void toast(tToast* toast);

private slots:
    void customContextMenu(QPoint pos);

private:
    DrivesModel* driveModel;

    QWidget* propertiesWidget;
};

#endif // DRIVESLIST_H
