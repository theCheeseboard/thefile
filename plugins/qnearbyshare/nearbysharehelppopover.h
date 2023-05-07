#ifndef NEARBYSHAREHELPPOPOVER_H
#define NEARBYSHAREHELPPOPOVER_H

#include <QWidget>

namespace Ui {
    class NearbyShareHelpPopover;
}

class NearbyShareHelpPopover : public QWidget {
        Q_OBJECT

    public:
        explicit NearbyShareHelpPopover(QWidget* parent = nullptr);
        ~NearbyShareHelpPopover();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

    private:
        Ui::NearbyShareHelpPopover* ui;
};

#endif // NEARBYSHAREHELPPOPOVER_H
