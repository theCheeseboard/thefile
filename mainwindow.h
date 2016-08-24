#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QDirIterator>
#include <QMimeDatabase>
#include <QProcess>
#include <QDateTime>
#include <QDBusInterface>
#include <QDBusReply>
#include <QXmlStreamReader>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QPainter>
#include <QStorageInfo>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QResizeEvent>
#include <QPropertyAnimation>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool isDirectory(QString dir);

private slots:
    void reloadList();

    void reloadLeftList();

    void on_currentDirBar_returnPressed();

    void on_actionGo_Up_triggered();

    void on_filesTable_cellActivated(int row, int column);

    void on_filesTable_cellDoubleClicked(int row, int column);

    bool navigate(QString path);

    void on_placesTable_itemActivated(QListWidgetItem *item);

    void on_actionNew_Window_triggered();

    void on_actionBookmark_triggered();

    void on_actionUnmount_triggered();

    void showErrorMessage(QString message);
    void showInfoMessage(QString message);

    void on_actionNew_Folder_triggered();

    void on_filesTable_itemChanged(QTableWidgetItem *item);

    void on_filesTable_customContextMenuRequested(const QPoint &pos);

    void on_actionRename_triggered();

    void on_filesTable_itemSelectionChanged();

    void on_actionShow_Hidden_Files_triggered(bool checked);

    void on_actionMove_To_Trash_triggered();

    void on_UndoButton_clicked();

    void restoreFilesFromTrash(QStringList filenames);

    void on_actionRestore_from_Trash_triggered();

    bool isTrashFolder();

    void on_actionDelete_Permanantly_triggered();

    void on_EmptyTrashButton_clicked();

    void on_actionMore_Info_triggered();

    void on_fileInfoDone_clicked();

    void setFilePermissions();

private:
    Ui::MainWindow *ui;

    QSettings settings;

    QFileSystemWatcher* watcher;

    QDir currentDir = QDir::home();
    QMap<QString, QString> MountPoints;
    QStringList pathHistory;

    QStringList FilesToUndo;

    QString renamingFile = "";

    QMimeDatabase mimeDatabase;

    void resizeEvent(QResizeEvent* event);
};

#endif // MAINWINDOW_H
