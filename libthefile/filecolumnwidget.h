#ifndef FILECOLUMNWIDGET_H
#define FILECOLUMNWIDGET_H

#include <QWidget>

class FileColumn;
struct FileColumnWidgetPrivate;
class FileColumnWidget : public QWidget {
        Q_OBJECT
    public:
        explicit FileColumnWidget(QWidget* parent = nullptr);
        ~FileColumnWidget();

        FileColumn* fileColumn();

    signals:

    protected:
        friend FileColumn;
        void setFileColumn(FileColumn* fileColumn);

    private:
        FileColumnWidgetPrivate* d;
};

#endif // FILECOLUMNWIDGET_H
