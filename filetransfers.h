#ifndef FILETRANSFERS_H
#define FILETRANSFERS_H

#include <QDialog>
#include <QTableWidget>

class copy;

namespace Ui {
class fileTransfers;
}

class fileTransfers : public QDialog
{
    Q_OBJECT

public:
    explicit fileTransfers(QWidget *parent = 0);
    ~fileTransfers();
    QTableWidget *table;

public slots:
    void showWin();
    void hideWin();

private:
    Ui::fileTransfers *ui;
};

#include "copy.h"
#endif // FILETRANSFERS_H
