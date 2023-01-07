#include "idevicesidebarsectionfactory.h"

#include "idevicesidebarsection.h"

struct IDeviceSidebarSectionFactoryPrivate {
        IDeviceWatcher* watcher;
};

IDeviceSidebarSectionFactory::IDeviceSidebarSectionFactory(IDeviceWatcher* watcher, QObject* parent) :
    SidebarSectionFactory{parent} {
    d = new IDeviceSidebarSectionFactoryPrivate();
    d->watcher = watcher;
}

IDeviceSidebarSectionFactory::~IDeviceSidebarSectionFactory() {
    delete d;
}

QList<SidebarSection*> IDeviceSidebarSectionFactory::init() {
    return {
        new IDeviceSidebarSection(d->watcher)};
}
