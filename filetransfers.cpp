#include "filetransfers.h"
#include "ui_filetransfers.h"

extern QList<copy*> copyops;
extern fileTransfers *transferWin;

fileTransfers::fileTransfers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileTransfers)
{
    ui->setupUi(this);

    table = ui->tableWidget;

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Source" << "Destination" << "Progress");
}

fileTransfers::~fileTransfers()
{
    delete ui;
}

void fileTransfers::showWin() {
    this->show();
}

void fileTransfers::hideWin() {
    this->hide();
}
