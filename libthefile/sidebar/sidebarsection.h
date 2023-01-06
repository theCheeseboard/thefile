#ifndef SIDEBARSECTION_H
#define SIDEBARSECTION_H

#include "directory.h"
#include <QObject>

struct SidebarSectionPrivate;
class SidebarSection : public QObject {
        Q_OBJECT
    public:
        explicit SidebarSection(QObject* parent = nullptr);
        ~SidebarSection();

        virtual QString label() = 0;
        virtual QWidget* widget() = 0;

        bool visible();
        void setVisibility(bool visible);

    signals:
        void labelChanged();
        void navigate(QUrl location);
        void moveFiles(QList<QUrl> source, DirectoryPtr destination);
        void copyFiles(QList<QUrl> source, DirectoryPtr destination);
        void visibilityChanged();

    private:
        SidebarSectionPrivate* d;
};

#endif // SIDEBARSECTION_H
