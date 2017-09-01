#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QPainter>
#include <QPaintEvent>
#include <tpropertyanimation.h>
#include <QLabel>
#include <QMimeDatabase>
#include <QDesktopWidget>
#include <QCheckBox>
#include <QPushButton>
#include <ttoast.h>

namespace Ui {
class PropertiesDialog;
}

class PropertiesDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit PropertiesDialog(QFileInfo fileInfo, QPoint pos, QWidget *parent = 0);
    ~PropertiesDialog();

    void setGeometry(QRect geometry);
    void setGeometry(int x, int y, int width, int height);

public slots:
    void show();

    void reloadPermissions();

    void reject();

private slots:
    void on_savePermissionsButton_clicked();

private:
    Ui::PropertiesDialog *ui;

    void paintEvent(QPaintEvent* event);
    QFileInfo info;
    QFile file;
};

#endif // PROPERTIESDIALOG_H
