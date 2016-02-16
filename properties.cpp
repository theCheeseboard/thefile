#include "properties.h"
#include "ui_properties.h"

Properties::Properties(QFile *file, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Properties)
{
    ui->setupUi(this);

    this->file = file;

}

Properties::~Properties()
{
    delete ui;
}
