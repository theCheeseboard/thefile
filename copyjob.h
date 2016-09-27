#ifndef COPYJOB_H
#define COPYJOB_H

#include <QFile>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QUrl>
#include <QDirIterator>
#include <QFontMetrics>
#include "runningtransfers.h"

class CopyJob : public QObject
{
    Q_OBJECT


public:
    CopyJob(QStringList source, QString destination);

private slots:
    void progressChanged(qulonglong bytesCopied, qulonglong bytesToCopy);
    void statusTextChanged(QString statusText);

signals:

private:
    QString destination;
    QStringList source;
    QThread* copyThread;

    QFrame* frame;
    QProgressBar* bar;
    QLabel* filesLabel;
};

class CopyJobWorker : public QObject
{
    Q_OBJECT
public:
    CopyJobWorker(QStringList source, QString destination);

public slots:
    void startCopy();

signals:
    void finishedCopy();
    void progressChanged(qulonglong bytesCopied, qulonglong bytesToCopy);
    void statusTextChanged(QString statusText);

private:
    QString destination;
    QStringList source;

    void copyFile(QString source, QString destination);

    qulonglong totalBytes = 0, copiedBytes = 0;

    QStringList ErrorFiles;
};

#endif // COPYJOB_H
