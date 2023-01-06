#include "sidebarmanager.h"

#include "sidebar/sections/standardsidebarsectionfactory.h"

struct SidebarManagerPrivate {
        QList<SidebarSectionFactory*> factories;
};

SidebarManager::SidebarManager(QObject* parent) :
    QObject{parent} {
    d = new SidebarManagerPrivate();
    d->factories.append(new StandardSidebarSectionFactory());
}

SidebarManager* SidebarManager::instance() {
    static auto* instance = new SidebarManager();
    return instance;
}

void SidebarManager::registerSidebarFactory(SidebarSectionFactory* factory) {
    instance()->d->factories.append(factory);
}

QList<SidebarSectionFactory*> SidebarManager::factories() {
    return instance()->d->factories;
}
