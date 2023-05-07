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

#include "filecolumn.h"
#include "filecolumnmanager.h"
#include <QDir>
#include <QListView>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <resourcemanager.h>
#include <tlogger.h>
#include <tvariantanimation.h>

struct FileTabPrivate {
        FileColumnManager* columnManager;
        DirectoryPtr currentDirectory;
        QList<DirectoryPtr> currentColumns;
        QList<FileColumn*> currentColumnWidgets;

        bool keepAtEnd = true;
};

FileTab::FileTab(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileTab) {
    ui->setupUi(this);
    d = new FileTabPrivate();
    d->columnManager = new FileColumnManager(this);
    connect(d->columnManager, &FileColumnManager::currentChanged, this, &FileTab::columnsChanged);

    ui->sidebar->setFixedWidth(SC_DPI(200));
    connect(ui->sidebar, &Sidebar::navigate, this, &FileTab::setCurrentUrl);
    connect(ui->sidebar, &Sidebar::moveFiles, this, &FileTab::moveFiles);
    connect(ui->sidebar, &Sidebar::copyFiles, this, &FileTab::copyFiles);

    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        d->keepAtEnd = ui->scrollArea->horizontalScrollBar()->maximum() == value;
        int max = 0;
        for (FileColumn* c : d->currentColumnWidgets) max += c->width();

        int margin = value + ui->scrollArea->width() - max;
        if (margin < 0) margin = 0;
        ui->scrollAreaWidgetContents->layout()->setContentsMargins(this->layoutDirection() == Qt::RightToLeft ? margin : 0, 0, this->layoutDirection() == Qt::LeftToRight ? margin : 0, 0);
    });
    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max) {
        if (d->keepAtEnd) {
            ui->scrollArea->horizontalScrollBar()->setValue(max);
        }
    });

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

    int currentWidth = ui->scrollArea->horizontalScrollBar()->value();

    d->currentDirectory = directory;

    QList<DirectoryPtr> directories;
    directories.append(directory);

    DirectoryPtr dir = ResourceManager::parentDirectoryForUrl(directories.last()->url());
    while (dir) {
        directories.append(dir);
        dir = ResourceManager::parentDirectoryForUrl(directories.last()->url());
    }

    switch (effectiveViewType()) {
        case Directory::ViewType::Column:
            ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
            break;
        case Directory::ViewType::Wide:
            ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            break;
    }

    std::reverse(directories.begin(), directories.end());

    bool columnsAdded = directories.count() > d->currentColumns.count();
    bool columnsRemoved = directories.count() < d->currentColumns.count();

    // Modify the current columns
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
            // We need to add a new column
            col = new FileColumn(directory, d->columnManager);
            col->setSelected(nextPath);
            connect(col, &FileColumn::navigate, this, &FileTab::setCurrentDir);
            connect(col, &FileColumn::directoryChanged, this, &FileTab::tabTitleChanged);
            connect(col, &FileColumn::canCopyCutTrashChanged, this, &FileTab::columnsChanged);
            connect(col, &FileColumn::copyFiles, this, &FileTab::copyFiles);
            connect(col, &FileColumn::moveFiles, this, &FileTab::moveFiles);
            connect(col, &FileColumn::deletePermanently, this, &FileTab::deletePermanently);
            connect(col, &FileColumn::openItemProperties, this, &FileTab::openItemProperties);
            connect(col, &FileColumn::burnDirectory, this, &FileTab::burnDirectory);
            ui->dirsLayout->addWidget(col);
            d->currentColumnWidgets.append(col);
        } else {
            col = d->currentColumnWidgets.at(i);
            col->setSelected(nextPath);
        }

        switch (effectiveViewType()) {
            case Directory::ViewType::Column:
                col->setFixedWidth(SC_DPI(300));
                break;
            case Directory::ViewType::Wide:
                col->setFixedWidth(QWIDGETSIZE_MAX);
                col->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                break;
        }
    }

    switch (effectiveViewType()) {
        case Directory::ViewType::Column:
            while (d->currentColumns.count() > directories.count()) {
                FileColumn* col = d->currentColumnWidgets.takeLast();
                ui->dirsLayout->removeWidget(col);
                col->deleteLater();
                d->currentColumns.removeLast();
            }

            if (columnsRemoved) {
                // Keep the scrollbar where it is
                // TODO: Adjust the scrollbar so that at least one panel is visible
                int max = -ui->scrollArea->width();
                for (FileColumn* c : qAsConst(d->currentColumnWidgets)) max += c->width();
                if (max < currentWidth) {
                    int margin = currentWidth - max;
                    ui->scrollAreaWidgetContents->layout()->setContentsMargins(this->layoutDirection() == Qt::RightToLeft ? margin : 0, 0, this->layoutDirection() == Qt::LeftToRight ? margin : 0, 0);
                }
            } else {
                // Reduce the extra margin at the end
                int max = 0;
                for (FileColumn* c : qAsConst(d->currentColumnWidgets)) max += c->width();

                int margin = ui->scrollArea->horizontalScrollBar()->value() + ui->scrollArea->width() - max;
                if (margin < 0) margin = 0;
                ui->scrollAreaWidgetContents->layout()->setContentsMargins(this->layoutDirection() == Qt::RightToLeft ? margin : 0, 0, this->layoutDirection() == Qt::LeftToRight ? margin : 0, 0);
            }
            if (columnsAdded) {
                QTimer::singleShot(0, this, [this] {
                    // Scroll to the end
                    tVariantAnimation* anim = new tVariantAnimation(this);
                    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, anim, [=](int min, int max) {
                        Q_UNUSED(min)
                        anim->setEndValue(max);
                    });
                    anim->setStartValue(ui->scrollArea->horizontalScrollBar()->value());
                    anim->setStartValue(ui->scrollArea->horizontalScrollBar()->maximum());
                    anim->setDuration(100);
                    anim->setEasingCurve(QEasingCurve::OutCubic);
                    connect(anim, &tVariantAnimation::valueChanged, this, [this](QVariant value) {
                        ui->scrollArea->horizontalScrollBar()->setValue(value.toInt());
                    });
                    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
                    anim->start();
                });
            }
            break;
        case Directory::ViewType::Wide:
            while (d->currentColumnWidgets.count() > 1) {
                FileColumn* col = d->currentColumnWidgets.takeLast();
                ui->dirsLayout->removeWidget(col);
                col->deleteLater();
            }
            while (directories.count() > 1) directories.removeLast();
            ui->scrollAreaWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);
            break;
    }

    d->currentColumns = directories;

    emit tabTitleChanged();
    emit columnsChanged();
}

QUrl FileTab::currentUrl() {
    if (d->currentColumns.isEmpty()) return QUrl();
    return d->currentColumns.last()->url();
}

void FileTab::setFileTransfersSupported(bool supported) {
    d->columnManager->setFileTransfersSupported(supported);
}

void FileTab::setCanOpenProperties(bool canOpen) {
    d->columnManager->setCanOpenProperties(canOpen);
}

void FileTab::setOpenFileButtons(QList<OpenFileButton> buttons) {
    d->columnManager->setOpenFileButtons(buttons);
}

void FileTab::setColumnActions(QList<ColumnAction> actions) {
    d->columnManager->setColumnActions(actions);
}

void FileTab::setFilters(QList<Filter> filters) {
    d->columnManager->setFilters(filters);
}

FileColumn* FileTab::currentColumn() {
    return d->columnManager->current();
}

FileColumn* FileTab::lastColumn() {
    for (auto widget = d->currentColumnWidgets.rbegin(); widget != d->currentColumnWidgets.rend(); widget++) {
        if (!(*widget)->isFile()) {
            return *widget;
        }
    }
    return nullptr;
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

Directory::ViewType FileTab::effectiveViewType() {
    return d->currentDirectory->viewType();
}
