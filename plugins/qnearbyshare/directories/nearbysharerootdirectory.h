#ifndef NEARBYSHAREROOTDIRECTORY_H
#define NEARBYSHAREROOTDIRECTORY_H

#include <directory.h>

class NearbyShareRootDirectory : public Directory {
        Q_OBJECT
    public:
        explicit NearbyShareRootDirectory(QObject* parent = nullptr);

    signals:

        // Directory interface
    public:
        QCoro::Task<bool> exists();
        bool isFile(QString path);
        QUrl url();
        quint64 listCount(QDir::Filters filters, QDir::SortFlags sortFlags);
        QCoro::Generator<FileInformation> list(QDir::Filters filters, QDir::SortFlags sortFlags, quint64 offset);
        QCoro::Task<FileInformation> fileInformation(QString filename);
        QCoro::Task<QIODevice*> open(QString filename, QIODeviceBase::OpenMode mode);
        QCoro::Task<> mkpath(QString filename);
        bool canTrash(QString filename);
        QCoro::Task<QUrl> trash(QString filename);
        QCoro::Task<> deleteFile(QString filename);
        bool canMove(QString filename, QUrl to);
        QCoro::Task<> move(QString filename, QUrl to);
        QVariant special(QString operation, QVariantMap args);
        FileColumnWidget* renderedWidget();
        ViewType viewType();
        QString columnTitle();
};

#endif // NEARBYSHAREROOTDIRECTORY_H
