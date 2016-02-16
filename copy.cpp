#include "copy.h"

extern QList<copy*> copyops;
//extern fileTransfers *transferWin;

copy::copy(QStringList *source, QString dest, MainWindow *parent) : QObject(parent)
{
    this->source = source;
    this->dest = dest;

    progressBar = new QProgressBar();

    QThread *thread = new QThread;
    copyWorker *worker = new copyWorker(source, dest);
    worker->moveToThread(thread);
    //connect(worker, SIGNAL (error(QString)), this, SLOT (errorString(QString)));
    connect(thread, SIGNAL (started()), worker, SLOT (process()));
    connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
    connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(copyFinished()));
    connect(worker, SIGNAL(progress(quint64,quint64,QString,QString)), this, SLOT(progress(quint64,quint64,QString,QString)));
    window = new transferWindow();
    connect(window, SIGNAL(cancelTransfer()), worker, SLOT(cancelTransfer()));
    connect(this, SIGNAL(prog(qint64,qint64,QString,QString)), window, SLOT(prog(qint64,qint64,QString,QString)));
    window->show();
    thread->start();

    //transferWin->show();
    //int rowIndex = transferWin->table->rowCount();
    //transferWin->table->setRowCount(transferWin->table->rowCount() + 1);
    //transferWin->table->setCellWidget(rowIndex, 2, progressBar);

}

void copy::copyFinished() {
    //transferWin->table->removeRow(transferWin->table);
    window->close();

}

void copy::copyError() {

}

void copy::progress(quint64 p, quint64 max, QString s, QString d) {
    //progressBar->setValue(prog);
    //progressBar->setValue(max);
    emit prog(p, max, s, d);
}
