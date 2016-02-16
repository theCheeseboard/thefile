#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QDialog>
#include <QFile>

namespace Ui {
class Properties;
}

class Properties : public QDialog
{
    Q_OBJECT

public:
    explicit Properties(QFile *file, QWidget *parent = 0);
    ~Properties();

private:
    Ui::Properties *ui;

    QFile *file;
};

#endif // PROPERTIES_H
