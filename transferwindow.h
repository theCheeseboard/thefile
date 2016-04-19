#ifndef TRANSFERWINDOW_H
#define TRANSFERWINDOW_H

#include <QDialog>
#include "copyworker.h"

namespace Ui {
class transferWindow;
}

class transferWindow : public QDialog
{
    Q_OBJECT

public:
    explicit transferWindow(QWidget *parent = 0);
    ~transferWindow();

signals:
    void cancelTransfer();

    void continueTransfer(copyWorker::continueTransferOptions option, bool applyToAll);

public slots:
    void prog(qint64 prog, qint64 max, QString name, QString dest);

    void fileConflict(QString filename);

private slots:
    void on_pushButton_clicked();

    void on_replaceFile_clicked();

    void on_skipFile_clicked();

    void on_cancelFile_clicked();

private:
    Ui::transferWindow *ui;
};

#endif // TRANSFERWINDOW_H
