#ifndef BOOKMARKSSIDEBARSECTION_H
#define BOOKMARKSSIDEBARSECTION_H

#include "../sidebarsection.h"

struct BookmarksSidebarSectionPrivate;
class BookmarksSidebarSection : public SidebarSection {
        Q_OBJECT
    public:
        explicit BookmarksSidebarSection(QObject* parent = nullptr);
        ~BookmarksSidebarSection();

    signals:

    private:
        BookmarksSidebarSectionPrivate* d;

        // SidebarSection interface
    public:
        QString label();
        QWidget* widget();
};

#endif // BOOKMARKSSIDEBARSECTION_H
