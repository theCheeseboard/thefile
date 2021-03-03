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
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <filecolumn.h>
#include <resourcemanager.h>
#include <tsettings.h>
#include "filetab.h"
#include "tabbutton.h"

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
    QString location = QInputDialog::getText(this, tr("Go"), tr("Enter a location to go to"), QLineEdit::Normal, text, &ok);
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
