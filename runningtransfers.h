#ifndef RUNNINGTRANSFERS_H
#define RUNNINGTRANSFERS_H

#include <QDialog>
#include <QFrame>

namespace Ui {
class RunningTransfers;
}

class RunningTransfers : public QDialog
{
    Q_OBJECT

public:
    explicit RunningTransfers(QWidget *parent = 0);
    ~RunningTransfers();

public slots:
    void AddFrame(QFrame* frame);
    void RemoveFrame(QFrame* frame);

private:
    Ui::RunningTransfers *ui;

    int numberOfTransfers = 0;
    int totalHeight = 0;
};

#endif // RUNNINGTRANSFERS_H
