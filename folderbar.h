#ifndef FOLDERBAR_H
#define FOLDERBAR_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QLineEdit>
#include <QStorageInfo>

class FolderBar : public QWidget
{
    Q_OBJECT
public:
    explicit FolderBar(QWidget *parent = nullptr);

signals:
    void go(QString path);

public slots:
    void setPath(QString path);

private slots:
    void pathEditEntered();

private:
    QBoxLayout* layout;
    QBoxLayout* buttonLayout;

    QLineEdit* pathEdit;

    QList<QPushButton*> buttons;
};

#endif // FOLDERBAR_H
