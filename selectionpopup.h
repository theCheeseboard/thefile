#ifndef SELECTIONPOPUP_H
#define SELECTIONPOPUP_H

#include <QWidget>

namespace Ui {
    class SelectionPopup;
}

class SelectionPopup : public QWidget
{
        Q_OBJECT

    public:
        explicit SelectionPopup(QWidget *parent = nullptr);
        ~SelectionPopup();

        void setItemText(QString itemText);

    private:
        Ui::SelectionPopup *ui;
};

#endif // SELECTIONPOPUP_H
