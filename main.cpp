#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("theFile");
    a.setOrganizationName("theSuite");

    MainWindow w;
    w.show();

    return a.exec();
}
