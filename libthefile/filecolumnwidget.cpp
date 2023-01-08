#include "filecolumnwidget.h"

struct FileColumnWidgetPrivate {
        FileColumn* fileColumn;
};

FileColumnWidget::FileColumnWidget(QWidget* parent) :
    QWidget{parent} {
    d = new FileColumnWidgetPrivate();
}

FileColumnWidget::~FileColumnWidget() {
    delete d;
}

FileColumn* FileColumnWidget::fileColumn() {
    return d->fileColumn;
}

void FileColumnWidget::setFileColumn(FileColumn* fileColumn) {
    d->fileColumn = fileColumn;
}
