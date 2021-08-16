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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tcsdtools.h>
#include <thelpmenu.h>
#include <tjobmanager.h>
#include <QUrl>
#include <QFileInfo>
#include <tinputdialog.h>
#include <QMessageBox>
#include <QShortcut>
#include <filecolumn.h>
#include <resourcemanager.h>
#include <tsettings.h>
#include "filetab.h"
#include "tabbutton.h"
#include "jobs/filetransferjob.h"
#include "popovers/deletepermanentlypopover.h"
#include "popovers/itempropertiespopover.h"
#include "popovers/burnpopover.h"
#include <tpopover.h>
#include <QDesktopServices>

struct MainWindowPrivate {
    tCsdTools csd;
    tSettings settings;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->resize(SC_DPI_T(this->size(), QSize));

    d = new MainWindowPrivate();
    d->csd.installMoveAction(ui->topWidget);
    d->csd.installResizeAction(this);

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    } else {
        ui->rightCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    }

    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actionNewTab);
    menu->addAction(ui->actionNew_Window);
    menu->addSeparator();
    menu->addAction(ui->actionCut);
    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);
    menu->addAction(ui->actionMove_to_Trash);
    menu->addSeparator();
    menu->addAction(ui->actionNew_Folder);
    menu->addAction(ui->actionSelect_All);
    menu->addSeparator();
    menu->addAction(ui->actionGo);
    menu->addAction(ui->actionShowHiddenFiles);
    menu->addSeparator();
    menu->addMenu(new tHelpMenu(this));
    menu->addAction(ui->actionCloseTab);
    menu->addAction(ui->actionExit);

    QShortcut* deleteShortcut = new QShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete), this);
    connect(deleteShortcut, &QShortcut::activated, this, [ = ] {
        FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
        if (tab->currentColumn()) tab->currentColumn()->deleteFile();
    });

#ifdef T_BLUEPRINT_BUILD
    ui->menuButton->setIcon(QIcon(":/icons/thefile-blueprint.svg"));
#else
    ui->menuButton->setIcon(QIcon::fromTheme("com.vicr123.thefile", QIcon(":/icons/thefile.svg")));
#endif

    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    ui->menuButton->setMenu(menu);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    ui->jobButtonLayout->addWidget(tJobManager::makeJobButton());

    connect(&d->settings, &tSettings::settingChanged, this, [ = ](QString key, QVariant value) {
        if (key == "View/HiddenFiles") ui->actionShowHiddenFiles->setChecked(value.toBool());
    });
    ui->actionShowHiddenFiles->setChecked(d->settings.value("View/HiddenFiles").toBool());

    updateMenuActions();
}

MainWindow::~MainWindow() {
    delete ui;
    delete d;
}

void MainWindow::newTab() {
    newTab(QUrl());
}

void MainWindow::newTab(QUrl url) {
    TabButton* button = new TabButton();
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    button->setCheckable(true);
    button->setAutoExclusive(true);
    ui->tabsLayout->addWidget(button);

    FileTab* tab = new FileTab();
    if (url.isValid()) tab->setCurrentUrl(url);
    ui->stackedWidget->addWidget(tab);

    button->setText(QFileInfo(QFileInfo(tab->currentUrl().path()).canonicalFilePath()).fileName());

    connect(button, &TabButton::clicked, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(tab);
    });
    connect(ui->stackedWidget, &tStackedWidget::switchingFrame, button, [ = ](int frame) {
        if (ui->stackedWidget->widget(frame) == tab) button->setChecked(true);
    });
    connect(ui->stackedWidget, &tStackedWidget::currentChanged, button, [ = ](int frame) {
        if (ui->stackedWidget->widget(frame) == tab) button->setChecked(true);
    });
    connect(tab, &FileTab::tabTitleChanged, this, [ = ] {
        button->setText(tab->tabTitle());
    });
    connect(tab, &FileTab::tabClosed, this, [ = ] {
        ui->stackedWidget->removeWidget(tab);
        ui->tabsLayout->removeWidget(button);
        button->deleteLater();
        tab->deleteLater();
    });
    connect(tab, &FileTab::columnsChanged, this, &MainWindow::updateMenuActions);
    connect(tab, &FileTab::moveFiles, this, [ = ](QList<QUrl> source, DirectoryPtr destination) {
        FileTransferJob* job = new FileTransferJob(FileTransferJob::Move, source, destination, this->window());
        tJobManager::trackJobDelayed(job);
    });
    connect(tab, &FileTab::copyFiles, this, [ = ](QList<QUrl> source, DirectoryPtr destination) {
        FileTransferJob* job = new FileTransferJob(FileTransferJob::Copy, source, destination, this->window());
        tJobManager::trackJobDelayed(job);
    });
    connect(tab, &FileTab::deletePermanently, this, [ = ](QList<QUrl> filesToDelete) {
        DeletePermanentlyPopover* jp = new DeletePermanentlyPopover(filesToDelete);
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &DeletePermanentlyPopover::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
        popover->show(this->window());
    });
    connect(tab, &FileTab::openItemProperties, this, [ = ](QUrl url) {
        ItemPropertiesPopover* jp = new ItemPropertiesPopover(url);
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &ItemPropertiesPopover::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
        popover->show(this->window());
    });
    connect(tab, &FileTab::burnDirectory, this, [ = ](DirectoryPtr dir) {
        BurnPopover* jp = new BurnPopover(dir);
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &BurnPopover::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &DeletePermanentlyPopover::deleteLater);
        popover->show(this->window());
    });
    tab->setFileTransfersSupported(true);
    tab->setCanOpenProperties(true);
    tab->setOpenFileButtons({
        {
            tr("Open"),
            QIcon::fromTheme("document-open"),
            [ = ](QList<QUrl> selected) {
                QDesktopServices::openUrl(selected.first());
            },
            true
        }
    });

    ui->stackedWidget->setCurrentWidget(tab);
}

void MainWindow::on_actionExit_triggered() {
    QApplication::exit();
}

void MainWindow::on_actionNewTab_triggered() {
    newTab();
}

void MainWindow::on_actionCloseTab_triggered() {
    if (ui->stackedWidget->count() == 1) {
        this->close();
    } else {
        static_cast<FileTab*>(ui->stackedWidget->currentWidget())->closeTab();
    }
}

void MainWindow::on_actionShowHiddenFiles_triggered(bool checked) {
    d->settings.setValue("View/HiddenFiles", checked);
}

void MainWindow::on_actionGo_triggered() {
    QUrl initialUrl = static_cast<FileTab*>(ui->stackedWidget->currentWidget())->currentUrl();
    QString text;
    if (initialUrl.isLocalFile()) {
        text = initialUrl.toLocalFile();
    } else {
        text = initialUrl.toString();
    }

    bool ok;
    QString location = tInputDialog::getText(this, tr("Go"), tr("Enter a location to go to"), QLineEdit::Normal, text, &ok);
    if (ok) {
        QUrl url = QUrl::fromUserInput(location);
//        if (ResourceManager::isDirectoryHandlerRegistered(url.scheme())) {
        static_cast<FileTab*>(ui->stackedWidget->currentWidget())->setCurrentUrl(url);
//        } else {
//            QMessageBox::warning(this, tr("Can't open that URL"), tr("%1 URLs are not supported").arg(url.scheme()));
//        }
    }
}

void MainWindow::on_actionCopy_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->currentColumn()) tab->currentColumn()->copy();
}

void MainWindow::on_actionCut_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->currentColumn()) tab->currentColumn()->cut();
}

void MainWindow::on_actionPaste_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->lastColumn()) tab->lastColumn()->paste();
}

void MainWindow::on_actionMove_to_Trash_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->currentColumn()) tab->currentColumn()->deleteOrTrash();
}

void MainWindow::updateMenuActions() {
    bool copyCutTrash = false;
    bool paste = false;

    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab) {
        FileColumn* current = tab->currentColumn();
        if (current) copyCutTrash = current->canCopyCutTrash();

        FileColumn* last = tab->lastColumn();
        if (last) paste = last->canPaste();
    }

    ui->actionCopy->setEnabled(copyCutTrash);
    ui->actionCut->setEnabled(copyCutTrash);
    ui->actionPaste->setEnabled(paste);
    ui->actionMove_to_Trash->setEnabled(copyCutTrash);
}

void MainWindow::on_stackedWidget_switchingFrame(int ) {
    updateMenuActions();
}

void MainWindow::on_actionSelect_All_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->currentColumn()) tab->currentColumn()->selectAll();
}

void MainWindow::on_actionNew_Window_triggered() {
    MainWindow* w = new MainWindow();
    w->newTab();
    w->show();
    w->activateWindow();
}

void MainWindow::on_actionNew_Folder_triggered() {
    FileTab* tab = static_cast<FileTab*>(ui->stackedWidget->currentWidget());
    if (tab->lastColumn()) tab->lastColumn()->newFolder();
}
