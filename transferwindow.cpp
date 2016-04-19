#include "transferwindow.h"
#include "ui_transferwindow.h"

transferWindow::transferWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::transferWindow)
{
    ui->setupUi(this);

    ui->conflictFrame->setVisible(false);
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

void transferWindow::fileConflict(QString filename) {
    ui->label_2->setText("What do you want to do with " + filename + "?");
    ui->infoFrame->setVisible(false);
    ui->conflictFrame->setVisible(true);
}

void transferWindow::on_replaceFile_clicked()
{
    emit continueTransfer(copyWorker::Overwrite, ui->applyToAllCheck->isChecked());
    ui->infoFrame->setVisible(true);
    ui->conflictFrame->setVisible(false);
}

void transferWindow::on_skipFile_clicked()
{
    emit continueTransfer(copyWorker::Skip, ui->applyToAllCheck->isChecked());
    ui->infoFrame->setVisible(true);
    ui->conflictFrame->setVisible(false);
}

void transferWindow::on_cancelFile_clicked()
{
    emit continueTransfer(copyWorker::Cancel, false);
    ui->infoFrame->setVisible(true);
    ui->conflictFrame->setVisible(false);
}
