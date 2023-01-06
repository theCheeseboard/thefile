#include "sidebarsection.h"

struct SidebarSectionPrivate {
        bool visible = true;
};

SidebarSection::SidebarSection(QObject* parent) :
    QObject{parent} {
    d = new SidebarSectionPrivate();
}

SidebarSection::~SidebarSection() {
    delete d;
}

bool SidebarSection::visible() {
    return d->visible;
}

void SidebarSection::setVisibility(bool visible) {
    d->visible = visible;
    emit visibilityChanged();
}
