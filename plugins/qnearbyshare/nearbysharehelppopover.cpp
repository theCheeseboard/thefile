#include "nearbysharehelppopover.h"
#include "ui_nearbysharehelppopover.h"

#include <tcontentsizer.h>

NearbyShareHelpPopover::NearbyShareHelpPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NearbyShareHelpPopover) {
    ui->setupUi(this);
    new tContentSizer(ui->helpContainer);
}

NearbyShareHelpPopover::~NearbyShareHelpPopover() {
    delete ui;
}

void NearbyShareHelpPopover::on_titleLabel_backButtonClicked() {
    emit done();
}
