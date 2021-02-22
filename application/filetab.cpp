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
#include "filecolumnmanager.h"
#include <QListView>
#include <tvariantanimation.h>
#include <QScrollBar>
#include <tlogger.h>
#include <resourcemanager.h>

struct FileTabPrivate {
    FileColumnManager* columnManager;
    DirectoryPtr currentDirectory;
    QList<DirectoryPtr> currentColumns;
    QList<FileColumn*> currentColumnWidgets;
};

FileTab::FileTab(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileTab) {
    ui->setupUi(this);
    d = new FileTabPrivate();
    d->columnManager = new FileColumnManager(this);

    ui->sidebar->setFixedWidth(SC_DPI(200));
    connect(ui->sidebar, &Sidebar::navigate, this, &FileTab::setCurrentUrl);

    setCurrentUrl(QUrl::fromLocalFile(QDir::homePath()));
}

FileTab::~FileTab() {
    delete ui;
    delete d;
}

void FileTab::setCurrentUrl(QUrl url) {
    setCurrentDir(ResourceManager::directoryForUrl(url));
}

void FileTab::setCurrentDir(DirectoryPtr directory) {
    if (!directory) return;

    d->currentDirectory = directory;

    QList<DirectoryPtr> directories;
    directories.append(directory);

    DirectoryPtr dir = ResourceManager::parentDirectoryForUrl(directories.last()->url());
    while (dir) {
        directories.append(dir);
        dir = ResourceManager::parentDirectoryForUrl(directories.last()->url());
    }

    switch (effectiveViewType()) {
        case FileTab::Columns:
            ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
            break;
        case FileTab::Trash:
            ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            break;
    }

    std::reverse(directories.begin(), directories.end());

    //Modify the current columns
    for (int i = 0; i < directories.count(); i++) {
        DirectoryPtr directory = directories.at(i);

        QUrl nextPath;
        if (i + 1 < directories.count()) {
            nextPath = directories.at(i + 1)->url();
        }

        FileColumn* col;
        if (d->currentColumns.count() > i && d->currentColumns.at(i)->url() != directory->url()) {
            col = d->currentColumnWidgets.at(i);
            col->setDirectory(directory);
            col->setSelected(nextPath);
        } else if (d->currentColumns.count() <= i) {
            //We need to add a new column
            col = new FileColumn(directory, d->columnManager);
            col->setSelected(nextPath);
            connect(col, &FileColumn::navigate, this, &FileTab::setCurrentDir);
            connect(col, &FileColumn::directoryChanged, this, &FileTab::tabTitleChanged);
            ui->dirsLayout->addWidget(col);
            d->currentColumnWidgets.append(col);
        } else {
            col = d->currentColumnWidgets.at(i);
            col->setSelected(nextPath);
        }

        switch (effectiveViewType()) {
            case FileTab::Columns:
                col->setFixedWidth(SC_DPI(300));
                break;
            case FileTab::Trash:
                col->setFixedWidth(QWIDGETSIZE_MAX);
                col->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                break;
        }
    }

    switch (effectiveViewType()) {
        case FileTab::Columns:
            while (d->currentColumns.count() > directories.count()) {
                FileColumn* col = d->currentColumnWidgets.takeLast();
                ui->dirsLayout->removeWidget(col);
                col->deleteLater();
                d->currentColumns.removeLast();
            }
            break;
        case FileTab::Trash:
            while (d->currentColumnWidgets.count() > 1) {
                FileColumn* col = d->currentColumnWidgets.takeLast();
                ui->dirsLayout->removeWidget(col);
                col->deleteLater();
            }
            while (directories.count() > 1) directories.removeLast();
            break;
    }

    d->currentColumns = directories;

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

    emit tabTitleChanged();
}

QUrl FileTab::currentUrl() {
    if (d->currentColumns.isEmpty()) return QUrl();
    return d->currentColumns.last()->url();
}

QString FileTab::tabTitle() {
    for (auto i = d->currentColumnWidgets.rbegin(); i != d->currentColumnWidgets.rend(); i++) {
        QString title = (*i)->columnTitle();
        if (!title.isEmpty()) return title;
    }
    return "";
}

void FileTab::closeTab() {
    emit tabClosed();
}

FileTab::ViewType FileTab::effectiveViewType() {
    if (d->currentDirectory->url().scheme() == "trash") {
        return Trash;
    } else {
        return Columns;
    }
}
