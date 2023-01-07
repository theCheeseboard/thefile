#ifndef IDEVICESIDEBARSECTIONFACTORY_H
#define IDEVICESIDEBARSECTIONFACTORY_H

#include <sidebar/sidebarsectionfactory.h>

class IDeviceWatcher;
struct IDeviceSidebarSectionFactoryPrivate;
class IDeviceSidebarSectionFactory : public SidebarSectionFactory {
        Q_OBJECT
    public:
        explicit IDeviceSidebarSectionFactory(IDeviceWatcher* watcher, QObject* parent = nullptr);
        ~IDeviceSidebarSectionFactory();

    signals:

    private:
        IDeviceSidebarSectionFactoryPrivate* d;

        // SidebarSectionFactory interface
    public:
        QList<SidebarSection*> init();
};

#endif // IDEVICESIDEBARSECTIONFACTORY_H
