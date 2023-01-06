#ifndef DRIVESSIDEBARSECTION_H
#define DRIVESSIDEBARSECTION_H

#include "../sidebarsection.h"
#include <QCoroTask>

class DiskObject;
struct DrivesSidebarSectionPrivate;
class DrivesSidebarSection : public SidebarSection {
        Q_OBJECT
    public:
        explicit DrivesSidebarSection(QObject* parent = nullptr);
        ~DrivesSidebarSection();

    signals:

    private:
        DrivesSidebarSectionPrivate* d;

        QCoro::Task<> mount(DiskObject* disk);

        // SidebarSection interface
    public:
        QString label();
        QWidget* widget();
};

#endif // DRIVESSIDEBARSECTION_H
