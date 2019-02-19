#ifndef SELECTIONPOPUP_H
#define SELECTIONPOPUP_H

#include <QWidget>
#include <QFileInfo>

namespace Ui {
    class SelectionPopup;
}

class SelectionPopup : public QWidget
{
        Q_OBJECT

    public:
        explicit SelectionPopup(QWidget *parent = nullptr);
        ~SelectionPopup();

        void setSelection(QList<QFileInfo> selection);

    signals:
        void clearSelection();

    private slots:
        void on_clearButton_clicked();

    private:
        Ui::SelectionPopup *ui;
};

#endif // SELECTIONPOPUP_H
