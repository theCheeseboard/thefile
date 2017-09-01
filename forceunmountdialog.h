#ifndef FORCEUNMOUNTDIALOG_H
#define FORCEUNMOUNTDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QListWidgetItem>

namespace Ui {
class ForceUnmountDialog;
}

class ForceUnmountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForceUnmountDialog(QString dir, QWidget *parent = 0);
    ~ForceUnmountDialog();

private:
    Ui::ForceUnmountDialog *ui;
};

#endif // FORCEUNMOUNTDIALOG_H
