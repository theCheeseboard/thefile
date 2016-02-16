#ifndef COPYWORKER_H
#define COPYWORKER_H
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <unistd.h>
#include <QDir>
#include <QDirIterator>

class copyWorker : public QObject
{
    Q_OBJECT
 public:
     copyWorker(QStringList* files, QString d);
     ~copyWorker();
 public slots:
     void process();
     void cancelTransfer();
 signals:
     void finished();
     void error(QString err);
     void progress(quint64 value, quint64 max, QString src, QString dest);
 private:
     QStringList *source;
     QString dest;
     bool cancelTransferNow = false;
};

#endif // COPYWORKER_H
