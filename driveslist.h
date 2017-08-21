#ifndef DRIVESLIST_H
#define DRIVESLIST_H

#include <QWidget>
#include <QListView>
#include "drivesmodel.h"

class DrivesList : public QListView
{
    Q_OBJECT
public:
    explicit DrivesList(QWidget *parent = nullptr);

signals:

public slots:

private:
    DrivesModel* driveModel;
};

#endif // DRIVESLIST_H
