#include "mainwindow.h"
#include <QApplication>
#include "transferengine.h"

TransferEngine* transferEngine;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theFile");

    transferEngine = new TransferEngine();

    MainWindow w;
    w.show();

    return a.exec();
}

QString calculateSize(quint64 size) {
    QString ret;
    if (size > 1152921504606846976) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024 / 1024 / 1024 / 1024), 'f', 2).append(" EiB");
    } else if (size > 1125899906842624) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024 / 1024 / 1024), 'f', 2).append(" PiB");
    } else if (size > 1099511627776) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024 / 1024), 'f', 2).append(" TiB");
    } else if (size > 1073741824) {
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
