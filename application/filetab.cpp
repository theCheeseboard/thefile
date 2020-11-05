/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#include "filetab.h"
#include "ui_filetab.h"

#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include "filecolumn.h"
#include <QListView>
#include <tvariantanimation.h>
#include <QScrollBar>
#include <tlogger.h>

struct FileTabPrivate {
    QUrl currentUrl;
    QList<QUrl> currentColumns;
    QList<FileColumn*> currentColumnWidgets;
};

FileTab::FileTab(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileTab) {
    ui->setupUi(this);
    d = new FileTabPrivate();

    ui->sidebar->setFixedWidth(SC_DPI(200));
    connect(ui->sidebar, &Sidebar::navigate, this, &FileTab::setCurrentUrl);

    setCurrentUrl(QUrl::fromLocalFile(QDir::homePath()));
}

FileTab::~FileTab() {
    delete ui;
    delete d;
}

void FileTab::setCurrentUrl(QUrl url) {
    url.setPath(url.path() + "/");
    url = url.resolved(QUrl("."));
    d->currentUrl = url;
    emit currentUrlChanged(d->currentUrl);

    QList<QUrl> columnPaths;
    QUrl bits = url;
    while (bits.path() != "/") {
        columnPaths.prepend(bits);
        bits = bits.resolved(QUrl(".."));
    }
    columnPaths.prepend(bits);

    //Modify the current columns
    for (int i = 0; i < columnPaths.count(); i++) {
        QUrl path = columnPaths.at(i);
        QUrl nextPath;

        if (i + 1 != columnPaths.count()) {
            nextPath = columnPaths.at(i + 1);
        }

        if (d->currentColumns.count() > i && d->currentColumns.at(i) != path) {
            FileColumn* col = d->currentColumnWidgets.at(i);
            col->setUrl(path);
            col->setSelected(nextPath);
        } else if (d->currentColumns.count() <= i) {
            //We need to add a new column
            FileColumn* col = new FileColumn(path);
            col->setFixedWidth(SC_DPI(300));
            col->setSelected(nextPath);
            connect(col, &FileColumn::navigate, this, &FileTab::setCurrentUrl);
            ui->dirsLayout->addWidget(col);
            d->currentColumnWidgets.append(col);
        } else {
            FileColumn* col = d->currentColumnWidgets.at(i);
            col->setSelected(nextPath);
        }
    }

    //TODO: remove extraenous columns
    while (d->currentColumns.count() > columnPaths.count()) {
        FileColumn* col = d->currentColumnWidgets.takeLast();
        ui->dirsLayout->removeWidget(col);
        col->deleteLater();
        d->currentColumns.removeLast();
    }

    d->currentColumns = columnPaths;

    //Scroll to the end
    tVariantAnimation* anim = new tVariantAnimation(this);
    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, anim, [ = ](int min, int max) {
        Q_UNUSED(min)
        anim->setEndValue(max);
    });
    anim->setStartValue(ui->scrollArea->horizontalScrollBar()->value());
    anim->setStartValue(ui->scrollArea->horizontalScrollBar()->maximum());
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        ui->scrollArea->horizontalScrollBar()->setValue(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    anim->start();
}

QUrl FileTab::currentUrl() {
    return d->currentUrl;
}

void FileTab::closeTab() {
    emit tabClosed();
}
