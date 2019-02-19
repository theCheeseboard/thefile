/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "transferpane.h"
#include "ui_transferpane.h"

#include "conflictresolver.h"

#include <tvariantanimation.h>

TransferPane::TransferPane(TransferObject* transfer, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TransferPane)
{
    ui->setupUi(this);

    this->transfer = transfer;
    this->setFixedWidth(400);

    if (transfer->transferType() == TransferObject::Copy) {
        ui->bigTitleLabel->setText(tr("Copying items").toUpper());
    } else {
        ui->bigTitleLabel->setText(tr("Moving items").toUpper());
    }
}

TransferPane::~TransferPane()
{
    delete ui;
}


void TransferPane::setActionLabelText(QString text) {
    ui->actionLabel->setText(text);
}

void TransferPane::resizeEvent(QResizeEvent *event) {
    emit heightChanged();
    QWidget::resizeEvent(event);
}

void TransferPane::progress(qulonglong value, qulonglong max) {
    ui->progressBar->setMaximum(max);
    ui->progressBar->setValue(value);
}

void TransferPane::resolveConflicts(FileConflictList conflicts, bool hasNonSimpleConflicts) {
    this->conflicts = conflicts;
    if (hasNonSimpleConflicts) {
        ui->examineAllConflicts->click();
    } else {
        ui->stack->setCurrentIndex(1);

        if (conflicts.count() == 1) {
            QFileInfo info(conflicts.first()->file);
            ui->conflictActionLabel->setText(tr("The file %1 already exists in the destination.").arg(info.fileName()));
        } else {
            ui->conflictActionLabel->setText(tr("%n file conflicts need to be resolved.", nullptr, conflicts.count()));
        }
        this->resize();
    }
}

QSize TransferPane::sizeHint() const {
    QSize size = QFrame::sizeHint();
    size.setHeight(ui->bigTitleLabel->sizeHint().height() + ui->stack->currentWidget()->sizeHint().height() + ui->line->sizeHint().height());
    return size;
}

void TransferPane::resize() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->height());
    anim->setEndValue(this->sizeHint().height());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        anim->setEndValue(this->sizeHint().height());
        this->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    anim->start();
}

void TransferPane::on_replaceAllConflicts_clicked()
{
    for (auto i = conflicts.begin(); i != conflicts.end(); i++) {
        i->data()->resolution = FileConflict::Overwrite;
    }
    emit conflictsResolved(conflicts);
    ui->stack->setCurrentIndex(0);
    resize();
}

void TransferPane::on_skipAllConflicts_clicked()
{
    for (auto i = conflicts.begin(); i != conflicts.end(); i++) {
        i->data()->resolution = FileConflict::Skip;
    }
    emit conflictsResolved(conflicts);
    ui->stack->setCurrentIndex(0);
    resize();
}

void TransferPane::on_cancelButton_clicked()
{
    ui->stack->setCurrentIndex(0);
    resize();
    emit cancelTransfer();
}

void TransferPane::on_examineAllConflicts_clicked()
{
    ui->stack->setCurrentIndex(2);
    for (QSharedPointer<FileConflict> conflict : conflicts) {
        ConflictResolver* resolver = new ConflictResolver(conflict);
        ui->conflictsLayout->addWidget(resolver);
        resolver->setVisible(true);
        resolvers.append(resolver);
    }

    resize();
}

bool TransferPane::needExtraHeight() {
    if (ui->stack->currentIndex() == 2) {
        return true;
    } else {
        return false;
    }
}

void TransferPane::on_skipAllButton_clicked()
{
    for (ConflictResolver* resolver : resolvers) {
        resolver->setResolution(FileConflict::Skip);
    }
}

void TransferPane::on_replaceAllButton_clicked()
{
    for (ConflictResolver* resolver : resolvers) {
        resolver->setResolution(FileConflict::Overwrite);
    }
}

void TransferPane::on_finishResolveButton_clicked()
{
    emit conflictsResolved(conflicts);
    ui->stack->setCurrentIndex(0);
    resize();
}

void TransferPane::transferFinished() {
    emit finished();
    this->setVisible(false);
}
