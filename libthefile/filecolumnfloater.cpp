/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "filecolumnfloater.h"
#include "ui_filecolumnfloater.h"

#include <QMouseEvent>
#include <QDrag>

struct FileColumnFloaterPrivate {
    FileColumn* parent;
    QModelIndexList indices;
};

FileColumnFloater::FileColumnFloater(FileColumn* parent) :
    QWidget(parent),
    ui(new Ui::FileColumnFloater) {
    ui->setupUi(this);
    d = new FileColumnFloaterPrivate();
    d->parent = parent;

    ui->actionsButton->installEventFilter(this);
}

FileColumnFloater::~FileColumnFloater() {
    delete ui;
    delete d;
}

void FileColumnFloater::setIndices(QModelIndexList indices) {
    d->indices = indices;
    if (indices.count() == 1) {
        ui->floaterText->setText(indices.at(0).data().toString());
    } else {
        ui->floaterText->setText(tr("%n items", nullptr, indices.count()));
    }
}

void FileColumnFloater::on_cutButton_clicked() {
    d->parent->cut();
}

void FileColumnFloater::on_copyButton_clicked() {
    d->parent->copy();
}

void FileColumnFloater::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !d->indices.isEmpty()) {
        const QAbstractItemModel* model = d->indices.at(0).model();
        QDrag* drag = new QDrag(this);
        drag->setMimeData(model->mimeData(d->indices));
        drag->exec(Qt::CopyAction);
    }
}

void FileColumnFloater::on_actionsButton_pressed() {
}

bool FileColumnFloater::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->actionsButton && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress)) {
        ui->actionsButton->setMenu(d->parent->menuForSelectedItems());
    }
    return false;
}
