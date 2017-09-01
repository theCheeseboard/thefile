#include "forceunmountdialog.h"
#include "ui_forceunmountdialog.h"

ForceUnmountDialog::ForceUnmountDialog(QString dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ForceUnmountDialog)
{
    ui->setupUi(this);

    QProcess lsofProc;
    lsofProc.start("lsof +f -F pcfn -- " + dir);
    lsofProc.waitForFinished();
    QString lsofOutput = lsofProc.readAll();

    QStringList lines = lsofOutput.split("\n");
    struct process {
        uint pid = 0;
        QString proc;
        QString file;
    };
    QList<process> processes;

    process currentProcess;
    for (QString line : lines) {
        if (line.startsWith("p")) {
            if (currentProcess.pid != 0) {
                processes.append(currentProcess);
            }

            currentProcess = process();
            currentProcess.pid = line.mid(1).toUInt();
        } else if (line.startsWith("c")) {
            currentProcess.proc = line.mid(1);
        } else if (line.startsWith("n")) {
            currentProcess.file = line.mid(1);
        }
    }
    processes.append(currentProcess);

    for (process proc : processes) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(proc.proc + " (PID " + QString::number(proc.pid) + ")");
        ui->usingApps->addItem(item);
    }
}

ForceUnmountDialog::~ForceUnmountDialog()
{
    delete ui;
}
