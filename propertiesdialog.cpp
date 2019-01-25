#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

#include <QClipboard>
#include <QMimeData>
#include "transfers/transferengine.h"

extern QString calculateSize(quint64 size);
extern TransferEngine* transferEngine;

PropertiesDialog::PropertiesDialog(QList<QFileInfo> info, QDir parentFolder, QPoint pos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog)
{
    ui->setupUi(this);
    this->info = info;
    this->parentFolder = parentFolder;

    this->setWindowFlags(Qt::Popup | Qt::CustomizeWindowHint);
    this->move(pos);

    ui->deleteButton->setProperty("type", "destructive");

    if (info.count() == 1) {
        QFileInfo i = info.first();
        ui->fileName->setText(i.fileName());
        ui->fileType->setText(tr("%1 file").arg(i.suffix()));
        ui->fileSize->setText(calculateSize(i.size()));

        QMimeDatabase mimedb;
        ui->fileIcon->setPixmap(QIcon::fromTheme(mimedb.mimeTypeForFile(i.filePath()).iconName(), QIcon::fromTheme("application-octet-stream")).pixmap(32, 32));
    } else {
        ui->fileName->setText(tr("%n item(s)", nullptr, info.count()));
        ui->fileType->setText(tr("Items"));
        ui->fileSize->setText("Something interesting");
        ui->fileIcon->setPixmap(QIcon::fromTheme("folder").pixmap(32, 32));
    }

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

    /*tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");

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
    anim->start();*/
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
    QFile file(info.first().filePath());

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
        toast->setTitle(tr("Permissions"));
        toast->setText(tr("That didn't work."));
        toast->show(this);
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    }
    file.flush();

    reloadPermissions();

    ui->stack->setCurrentIndex(0);
}

void PropertiesDialog::reloadPermissions() {
    if (info.count() == 1) {
        ui->permissionsFrame->setVisible(true);
        //QFile file(this->file.first());
        QFile file(info.first().filePath());
        //file.first().setFileName(info.first().filePath());
        ui->ownerRead->setChecked(file.permissions() & QFile::ReadOwner);
        ui->ownerWrite->setChecked(file.permissions() & QFile::WriteOwner);
        ui->ownerExe->setChecked(file.permissions() & QFile::ExeOwner);
        ui->groupRead->setChecked(file.permissions() & QFile::ReadGroup);
        ui->groupWrite->setChecked(file.permissions() & QFile::WriteGroup);
        ui->groupExe->setChecked(file.permissions() & QFile::ExeGroup);
        ui->otherRead->setChecked(file.permissions() & QFile::ReadOther);
        ui->otherWrite->setChecked(file.permissions() & QFile::WriteOther);
        ui->otherExe->setChecked(file.permissions() & QFile::ExeOther);
    } else {
        ui->permissionsFrame->setVisible(false);
    }
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

void PropertiesDialog::on_copyButton_clicked()
{
    QString data = "copy\n";
    QList<QUrl> urls;
    for (QFileInfo i : this->info) {
        QUrl url = QUrl::fromLocalFile(i.filePath());
        data.append(url.toString() + "\n");
        urls.append(url.toString());
    }

    QMimeData* d = new QMimeData;
    d->setData("x-special/gnome-copied-files", data.toUtf8());
    d->setData("application/x-kde-cutselection", "0");
    d->setUrls(urls);

    QApplication::clipboard()->setMimeData(d);

    this->close();
}

void PropertiesDialog::on_permissionsButton_clicked()
{
    ui->stack->setCurrentIndex(1);
}

void PropertiesDialog::on_pasteButton_clicked()
{
    const QMimeData* d = QApplication::clipboard()->mimeData();

    QByteArray ba = d->data("text/uri-list");
    QStringList files = QString(ba).split("\r\n");
    TransferObject::TransferType type;
    if (d->data("application/x-kde-cutselection") == "0") {
        type = TransferObject::Copy;
    } else {
        type = TransferObject::Move;
    }

    TransferObject* transfer = new TransferObject(files, parentFolder.absolutePath(), type);
    transferEngine->addTransfer(transfer);

    this->close();
}
