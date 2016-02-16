#ifndef TRANSFERWINDOW_H
#define TRANSFERWINDOW_H

#include <QDialog>

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

public slots:
    void prog(qint64 prog, qint64 max, QString name, QString dest);

private slots:
    void on_pushButton_clicked();

private:
    Ui::transferWindow *ui;
};

#endif // TRANSFERWINDOW_H
