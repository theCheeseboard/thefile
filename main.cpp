#include "mainwindow.h"
#include "runningtransfers.h"
#include <QApplication>

RunningTransfers* TransferWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("theFile");
    a.setOrganizationName("theSuite");

    TransferWin = new RunningTransfers(0);

    MainWindow w;
    w.show();

    return a.exec();
}


QString calculateSize(quint64 size) {
    QString ret;
    if (size > 1073741824) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024), 'f', 2).append(" GiB");
    } else if (size > 1048576) {
        ret = QString::number(((float) size / 1024 / 1024), 'f', 2).append(" MiB");
    } else if (size > 1024) {
        ret = QString::number(((float) size / 1024), 'f', 2).append(" KiB");
    } else {
        ret = QString::number((float) size, 'f', 2).append(" B");
    }

    return ret;
}
