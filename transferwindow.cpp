#include "transferwindow.h"
#include "ui_transferwindow.h"

transferWindow::transferWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::transferWindow)
{
    ui->setupUi(this);

    ui->conflictFrame->setEnabled(false);
}

transferWindow::~transferWindow()
{
    delete ui;
}

void transferWindow::prog(qint64 prog, qint64 max, QString name, QString dest) {
    ui->progressBar->setMaximum(max);
    ui->progressBar->setValue(prog);
    ui->allFileSizes->setText("Size of all files: " + QString::number(max) + " B");
    ui->fileSizes->setText("Transferred: " + QString::number(prog) + " B");
    ui->copyFrom->setText("Currently copying " + name);
    ui->copyTo->setText("Copying to " + dest);
}

void transferWindow::on_pushButton_clicked()
{
    emit cancelTransfer();
}
