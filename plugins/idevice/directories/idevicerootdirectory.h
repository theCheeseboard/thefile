#ifndef IDEVICEROOTDIRECTORY_H
#define IDEVICEROOTDIRECTORY_H

#include <directory.h>

class IDevice;
struct IDeviceRootDirectoryPrivate;
class IDeviceRootDirectory : public Directory {
        Q_OBJECT
    public:
        explicit IDeviceRootDirectory(IDevice* device, QObject* parent = nullptr);
        ~IDeviceRootDirectory();

    signals:

    private:
        IDeviceRootDirectoryPrivate* d;

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
        QList<FileColumnWidget*> actions();
};

#endif // IDEVICEROOTDIRECTORY_H
