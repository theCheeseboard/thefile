#ifndef COPYWORKER_H
#define COPYWORKER_H
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <unistd.h>
#include <QDir>
#include <QDirIterator>
#include <QApplication>

class copyWorker : public QObject
{
    Q_OBJECT
 public:
     enum continueTransferOptions {
         Waiting,
         Overwrite,
         Skip,
         Cancel
     };

     copyWorker(QStringList files, QString d, bool deleteOriginal = false);
     ~copyWorker();
 public slots:
     void process();
     void cancelTransfer();
     void continueTransfer(copyWorker::continueTransferOptions option, bool applyToAll);
 signals:
     void finished();
     void error(QString err);
     void progress(quint64 value, quint64 max, QString src, QString dest);
     void fileConflict(QString filename);
 private:
     QStringList source;
     QString dest;
     bool cancelTransferNow = false;
     bool deleteOriginal;

     continueTransferOptions continueOption;
     bool continueOptionStay = false;
};

Q_DECLARE_METATYPE(copyWorker::continueTransferOptions)

#endif // COPYWORKER_H
