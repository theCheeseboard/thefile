#ifndef IDEVICESIDEBARSECTION_H
#define IDEVICESIDEBARSECTION_H

#include <sidebar/sidebarsection.h>

class IDeviceWatcher;
struct IDeviceSidebarSectionPrivate;
class IDeviceSidebarSection : public SidebarSection {
        Q_OBJECT
    public:
        explicit IDeviceSidebarSection(IDeviceWatcher* watcher, QObject* parent = nullptr);
        ~IDeviceSidebarSection();

    signals:

    private:
        IDeviceSidebarSectionPrivate* d;

        // SidebarSection interface
    public:
        QString label();
        QWidget* widget();
};

#endif // IDEVICESIDEBARSECTION_H
