#include "mainwindow.h"
#include "copyworker.h"
#include <QApplication>

extern fileTransfers *transferWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<copyWorker::continueTransferOptions>("copyWorker::continueTransferOptions");

    if (QIcon::themeName() == "hicolor") {
        QIcon::setThemeName("breeze");
    }

    MainWindow w;
    if (a.arguments().count() > 1) {
        w.goTo(a.arguments().at(1));
    }
    w.show();

    transferWin = new fileTransfers();

    return a.exec();
}
