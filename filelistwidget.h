#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QTableWidget>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>

class fileListWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit fileListWidget(QWidget *parent = 0);

signals:
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // FILELISTWIDGET_H
