#ifndef COPY_H
#define COPY_H
#include "mainwindow.h"
#include "copyworker.h"
#include "filetransfers.h"
#include "transferwindow.h"

#include <QStringList>
#include <QThread>
#include <QProgressBar>

class MainWindow;
class copy : public QObject
{
    Q_OBJECT
public:
    copy(QStringList source, QString dest, bool deleteOriginal = false, MainWindow *parent = 0);

signals:
    void showTransferWin();
    void hideTransferWin();

    void prog(qint64 prog, qint64 max, QString name, QString dest);

public slots:
    void copyFinished();
    void copyError();
    void progress(quint64 prog, quint64 max, QString s, QString d);

private:
    QStringList source;
    QString dest;
    transferWindow *window;

    QProgressBar *progressBar;
};

#endif // COPY_H
