#ifndef SIDEBARSECTIONFACTORY_H
#define SIDEBARSECTIONFACTORY_H

#include "sidebarsection.h"

class SidebarSectionFactory : public QObject
{
    Q_OBJECT
public:
    explicit SidebarSectionFactory(QObject *parent = nullptr);

    virtual QList<SidebarSection*> init() = 0;

signals:

};

#endif // SIDEBARSECTIONFACTORY_H
