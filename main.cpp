#include "mainwindow.h"
#include <QApplication>

extern fileTransfers *transferWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    transferWin = new fileTransfers();

    return a.exec();
}
