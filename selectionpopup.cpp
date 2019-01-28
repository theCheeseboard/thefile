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

void SelectionPopup::setSelection(QList<QFileInfo> selection) {
    ui->numSelected->setText(tr("%n item(s) selected", nullptr, selection.count()));
}

void SelectionPopup::on_clearButton_clicked()
{
    emit clearSelection();
}
