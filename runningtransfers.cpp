#include "runningtransfers.h"
#include "ui_runningtransfers.h"

RunningTransfers::RunningTransfers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunningTransfers)
{
    ui->setupUi(this);
}

RunningTransfers::~RunningTransfers()
{
    delete ui;
}

void RunningTransfers::AddFrame(QFrame *frame) {
    totalHeight += frame->sizeHint().height();
    ui->RunningTransferAreaLayout->layout()->addWidget(frame);

    if (++numberOfTransfers > 0) {
        this->show();

        int height = totalHeight;
        if (height > 300) height = 300;
        this->setFixedHeight(height + ui->titleLabel->sizeHint().height() + 30);
    }
}

void RunningTransfers::RemoveFrame(QFrame *frame) {
    totalHeight -= frame->sizeHint().height();
    ui->RunningTransferAreaLayout->layout()->removeWidget(frame);

    if (--numberOfTransfers == 0) {
        this->hide();
    } else {
        int height = totalHeight;
        if (height > 300) height = 300;
        this->setFixedHeight(height + ui->titleLabel->sizeHint().height() + 30);
    }
}
