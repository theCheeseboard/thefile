#include "filelistwidget.h"

fileListWidget::fileListWidget(QWidget *parent) : QTableWidget(parent)
{

}

void fileListWidget::mousePressEvent(QMouseEvent *event) {
    emit mousePress(event);
    event->accept();

}

void fileListWidget::mouseMoveEvent(QMouseEvent *event) {
    emit mouseMove(event);
}
