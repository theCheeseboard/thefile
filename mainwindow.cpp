#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    folderBar = new FolderBar();
    ui->toolbar->addWidget(folderBar);
    connect(folderBar, &FolderBar::go, [=](QString path) {
        currentTable()->go(path);
    });

    switch (settings.value("view/type", FileTable::List).toInt()) {
        case FileTable::List:
            ui->actionAs_List->setChecked(true);
            break;
        case FileTable::Tree:
            ui->actionAs_Tree->setChecked(true);
            break;
    }

    addTab();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addTab(QString directory) {
    FileTable* table = new FileTable(directory);
    ui->fileTables->addTab(table, "Files");
    connect(table, &FileTable::titleChanged, [=](QString title) {
        ui->fileTables->setTabText(ui->fileTables->indexOf(table), title);
    });
    connect(table, &FileTable::pathChanged, [=](QString path) {
        if (currentTable() == table) {
            folderBar->setPath(path);
        }
    });
}

void MainWindow::on_actionNew_Tab_triggered()
{
    addTab();
}

void MainWindow::on_actionGo_Up_triggered()
{
    currentTable()->goUp();
}

FileTable* MainWindow::currentTable() {
    return (FileTable*) ui->fileTables->currentWidget();
}

void MainWindow::on_actionAs_List_triggered()
{
    ui->actionAs_Tree->setChecked(false);
    ui->actionAs_List->setChecked(true);
    settings.setValue("view/type", FileTable::List);

    for (int i = 0; i < ui->fileTables->count(); i++) {
        ((FileTable*) ui->fileTables->widget(i))->setViewType(FileTable::List);
    }
}

void MainWindow::on_actionAs_Tree_triggered()
{
    ui->actionAs_Tree->setChecked(true);
    ui->actionAs_List->setChecked(false);
    settings.setValue("view/type", FileTable::Tree);

    for (int i = 0; i < ui->fileTables->count(); i++) {
        ((FileTable*) ui->fileTables->widget(i))->setViewType(FileTable::Tree);
    }
}

void MainWindow::on_fileTables_currentChanged(int index)
{
    FileTable* table = (FileTable*) ui->fileTables->widget(index);
    folderBar->setPath(table->path());
    ui->actionHidden_Files->setChecked(table->showingHidden());
}

void MainWindow::on_actionRename_triggered()
{
    currentTable()->rename();
}

void MainWindow::on_actionDelete_triggered()
{
    currentTable()->rm();
}

void MainWindow::on_actionNew_Folder_triggered()
{
    currentTable()->mkdir(currentTable()->rootIndex());
}

void MainWindow::on_drives_activated(const QModelIndex &index)
{
    PaneItem item = ui->drives->model()->data(index, Qt::UserRole).value<PaneItem>();
    if (item.itemType() == PaneItem::Disk) {
        QList<QString> mountPoints = item.mountPoints();

        if (mountPoints.count() == 0) {
            QString path = item.attemptMount();
            if (path.startsWith("/")) {
                currentTable()->go(path);
            } else {
                tToast* toast = new tToast();
                toast->setTitle("Mount Error");
                toast->setText(path);
                toast->show(this);
                connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            }
        } else {
            currentTable()->go(mountPoints.first());
        }
    } else if (item.itemType() == PaneItem::Filesystem) {
        currentTable()->go(item.filePath());
    }
}

void MainWindow::on_actionAbout_theFile_triggered()
{
    AboutDialog dlg;
    dlg.exec();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionHidden_Files_toggled(bool arg1)
{
    currentTable()->setShowHidden(arg1);
}

void MainWindow::on_drives_toast(tToast *toast)
{
    toast->show(this);
}

void MainWindow::on_fileTables_tabCloseRequested(int index)
{
    ui->fileTables->widget(index)->deleteLater();
}
