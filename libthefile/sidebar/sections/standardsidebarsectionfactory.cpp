#include "standardsidebarsectionfactory.h"

#include "bookmarkssidebarsection.h"
#include "drivessidebarsection.h"
#include "placessidebarsection.h"

StandardSidebarSectionFactory::StandardSidebarSectionFactory(QObject* parent) :
    SidebarSectionFactory{parent} {
}

QList<SidebarSection*> StandardSidebarSectionFactory::init() {
    return {
        new PlacesSidebarSection(), new DrivesSidebarSection(), new BookmarksSidebarSection()};
}
