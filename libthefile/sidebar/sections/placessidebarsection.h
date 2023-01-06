#ifndef PLACESSIDEBARSECTION_H
#define PLACESSIDEBARSECTION_H

#include "../sidebarsection.h"

struct PlacesSidebarSectionPrivate;
class PlacesSidebarSection : public SidebarSection {
        Q_OBJECT
    public:
        explicit PlacesSidebarSection(QObject* parent = nullptr);
        ~PlacesSidebarSection();

    signals:

    private:
        PlacesSidebarSectionPrivate* d;

        // SidebarSection interface
    public:
        QString label();
        QWidget* widget();

        // QObject interface
    public:
        bool eventFilter(QObject *watched, QEvent *event);
};

#endif // PLACESSIDEBARSECTION_H
