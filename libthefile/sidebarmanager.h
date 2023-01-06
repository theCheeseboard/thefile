#ifndef SIDEBARMANAGER_H
#define SIDEBARMANAGER_H

#include <QObject>

class SidebarSectionFactory;
struct SidebarManagerPrivate;
class SidebarManager : public QObject {
        Q_OBJECT
    public:
        explicit SidebarManager(QObject* parent = nullptr);

        static SidebarManager* instance();

        static void registerSidebarFactory(SidebarSectionFactory* factory);
        static QList<SidebarSectionFactory*> factories();

    signals:

    private:
        SidebarManagerPrivate* d;
};

#endif // SIDEBARMANAGER_H
