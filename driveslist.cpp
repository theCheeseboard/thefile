#include "driveslist.h"

DrivesList::DrivesList(QWidget *parent) : QListView(parent)
{
    driveModel = new DrivesModel();
    this->setModel(driveModel);
    this->setItemDelegate(new DrivesDelegate);
}
