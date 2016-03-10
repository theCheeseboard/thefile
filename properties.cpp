#include "properties.h"
#include "ui_properties.h"

extern QString calculateSize(quint64 size);

Properties::Properties(QFile *file, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Properties)
{
    ui->setupUi(this);

    this->file = file;
    QFileInfo info(*file);
    ui->filename->setText(info.fileName());
    ui->size->setText(calculateSize(info.size()));
    ui->created->setText(info.created().toString());
    ui->modified->setText(info.lastModified().toString());


}

Properties::~Properties()
{
    delete ui;
}

void Properties::on_pushButton_clicked()
{
    this->close();
}
