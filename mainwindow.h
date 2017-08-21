#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QAction>
#include <QToolBar>
#include "filetable.h"
#include "folderbar.h"
#include "aboutdialog.h"
#include <ttoast.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void addTab(QString directory = QDir::homePath());

private slots:
    void on_actionNew_Tab_triggered();

    void on_actionGo_Up_triggered();

    FileTable* currentTable();

    void on_actionAs_List_triggered();

    void on_actionAs_Tree_triggered();

    void on_fileTables_currentChanged(int index);

    void on_actionRename_triggered();

    void on_actionDelete_triggered();

    void on_actionNew_Folder_triggered();

    void on_drives_activated(const QModelIndex &index);

    void on_actionAbout_theFile_triggered();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;

    FolderBar* folderBar;

    QSettings settings;
};

#endif // MAINWINDOW_H
