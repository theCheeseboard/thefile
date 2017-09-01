#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

extern QString calculateSize(quint64 size);

PropertiesDialog::PropertiesDialog(QFileInfo info, QPoint pos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog)
{
    ui->setupUi(this);
    this->info = info;
    this->file.setFileName(info.filePath());

    this->setWindowFlags(Qt::Popup | Qt::CustomizeWindowHint);
    this->move(pos);

    ui->fileName->setText(info.fileName());
    ui->fileType->setText(info.suffix() + " file");
    ui->fileSize->setText(calculateSize(info.size()));
    ui->savePermissionsButton->hide();
    ui->deleteButton->setProperty("type", "destructive");

    QMimeDatabase mimedb;
    ui->fileIcon->setPixmap(QIcon::fromTheme(mimedb.mimeTypeForFile(info.filePath()).iconName(), QIcon::fromTheme("application-octet-stream")).pixmap(32, 32));

    reloadPermissions();
}

PropertiesDialog::~PropertiesDialog()
{
    delete ui;
}

void PropertiesDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawRect(QRect(0, 0, this->width() - 1, this->height() - 1));
}

void PropertiesDialog::show() {
    QDialog::show();

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");

    if (this->geometry().bottom() > QApplication::desktop()->screenGeometry(this->geometry().topLeft()).bottom()) {
        anim->setStartValue(QRect(this->pos().x(), this->pos().y() + 15, this->width(), 0));
        anim->setEndValue(QRect(this->pos().x(), this->pos().y() - this->height(), this->width(), this->height()));
        ((QBoxLayout*) this->layout())->setDirection(QBoxLayout::BottomToTop);
    } else {
        anim->setStartValue(QRect(this->pos().x(), this->pos().y() - 15, this->width(), 0));
        anim->setEndValue(QRect(this->pos(), this->size()));
        ((QBoxLayout*) this->layout())->setDirection(QBoxLayout::TopToBottom);
    }
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void PropertiesDialog::setGeometry(QRect geometry) {
    this->setFixedWidth(geometry.width());
    this->setFixedHeight(geometry.height());
    this->move(geometry.topLeft());
}

void PropertiesDialog::setGeometry(int x, int y, int width, int height) {
    setGeometry(QRect(x, y, width, height));
}

void PropertiesDialog::on_savePermissionsButton_clicked()
{
    QFile::Permissions permissions;
    if (ui->ownerRead->isChecked()) permissions |= QFile::ReadOwner;
    if (ui->ownerWrite->isChecked()) permissions |= QFile::WriteOwner;
    if (ui->ownerExe->isChecked()) permissions |= QFile::ExeOwner;
    if (ui->groupRead->isChecked()) permissions |= QFile::ReadGroup;
    if (ui->groupWrite->isChecked()) permissions |= QFile::WriteGroup;
    if (ui->groupExe->isChecked()) permissions |= QFile::ExeGroup;
    if (ui->otherRead->isChecked()) permissions |= QFile::ReadOther;
    if (ui->otherWrite->isChecked()) permissions |= QFile::WriteOther;
    if (ui->otherExe->isChecked()) permissions |= QFile::ExeOther;
    if (!file.setPermissions(permissions)) {
        tToast* toast = new tToast();
        toast->setTitle("Permissions");
        toast->setText("That didn't work.");
        toast->show(this);
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    }
    ui->savePermissionsButton->hide();
    file.flush();

    reloadPermissions();
}

void PropertiesDialog::reloadPermissions() {
    file.setFileName(info.filePath());
    ui->ownerRead->setChecked(file.permissions() & QFile::ReadOwner);
    ui->ownerWrite->setChecked(file.permissions() & QFile::WriteOwner);
    ui->ownerExe->setChecked(file.permissions() & QFile::ExeOwner);
    ui->groupRead->setChecked(file.permissions() & QFile::ReadGroup);
    ui->groupWrite->setChecked(file.permissions() & QFile::WriteGroup);
    ui->groupExe->setChecked(file.permissions() & QFile::ExeGroup);
    ui->otherRead->setChecked(file.permissions() & QFile::ReadOther);
    ui->otherWrite->setChecked(file.permissions() & QFile::WriteOther);
    ui->otherExe->setChecked(file.permissions() & QFile::ExeOther);
}

void PropertiesDialog::reject() {/*
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
    QDialog::show();
    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(this->geometry().adjusted(0, 0, 0, -this->geometry().height()));
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->setDuration(500);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tPropertyAnimation::finished, [=] {
        QDialog::reject();
    });
    anim->start();*/
    QDialog::reject();
}
