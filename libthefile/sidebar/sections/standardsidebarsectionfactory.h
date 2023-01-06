#ifndef STANDARDSIDEBARSECTIONFACTORY_H
#define STANDARDSIDEBARSECTIONFACTORY_H

#include "../sidebarsectionfactory.h"

class StandardSidebarSectionFactory : public SidebarSectionFactory {
        Q_OBJECT
    public:
        explicit StandardSidebarSectionFactory(QObject* parent = nullptr);

    signals:

        // SidebarSectionFactory interface
    public:
        QList<SidebarSection*> init();
};

#endif // STANDARDSIDEBARSECTIONFACTORY_H
