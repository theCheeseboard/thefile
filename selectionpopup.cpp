#include "selectionpopup.h"
#include "ui_selectionpopup.h"

#include <the-libs_global.h>

SelectionPopup::SelectionPopup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectionPopup)
{
    ui->setupUi(this);

    this->resize(this->size() * theLibsGlobal::getDPIScaling());
}

SelectionPopup::~SelectionPopup()
{
    delete ui;
}

void SelectionPopup::setItemText(QString itemText) {
    ui->numSelected->setText(itemText);
}
