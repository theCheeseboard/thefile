#include "mainwindow.h"
#include "ui_mainwindow.h"

QString calculateSize(quint64 size) {
    QString ret;
    if (size > 1073741824) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024), 'f', 2).append(" GiB");
    } else if (size > 1048576) {
        ret = QString::number(((float) size / 1024 / 1024), 'f', 2).append(" MiB");
    } else if (size > 1024) {
        ret = QString::number(((float) size / 1024), 'f', 2).append(" KiB");
    } else {
        ret = QString::number((float) size, 'f', 2).append(" B");
    }

    return ret;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->mainToolBar->addWidget(ui->addr);
    ui->message->setVisible(false);
    ui->messageFav->setVisible(false);

    if (copyops.count() == 0) {
        copyops = QList<copy*>();
    }

    if (transferWin == 0) {
        transferWin = new fileTransfers();
    }

    /*udisks = new QDBusInterface(
                "org.freedesktop.UDisks",
                "here comes the path from the QDBusObjectPath.path() object",
                "org.freedesktop.UDisks.Device",
                QDBusConnection::systemBus(),
                this
            );*//*
    udisks = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus(), this);
    QDBusMessage call = QDBusMessage::createMethodCall("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.Properties", "GetAll");

    QList<QVariant> args;
    args.append("org.freedesktop.UDisks.Filesystem");
    call.setArguments(args);

    QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();

    QVariantMap map = reply.value();
    for (QVariant m : map) {

    }*/

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadList()));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadList()));

    icons = new QFileIconProvider();

    ui->files->setColumnCount(4);
    ui->files->setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Size" << "Date");
    ui->files->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->files->setMouseTracking(true);
    ui->files->viewport()->setMouseTracking(true);
    ui->files->installEventFilter(this);
    ui->files->viewport()->installEventFilter(this);

    QDir::home().mkdir(".thefile");
    QFile fav(QDir::homePath() + "/.thefile/fav");
    if (!fav.exists()) {
        fav.open(QFile::WriteOnly);
        fav.write(QString(QDir::homePath() + ",Home\n").toUtf8());
        fav.close();
    }

    udisks = new UDisks2(this);
    connect(udisks, SIGNAL(blockDeviceAdded(QString)), this, SLOT(blockDevicesChanged()));
    connect(udisks, SIGNAL(blockDeviceChanged(QString)), this, SLOT(blockDevicesChanged()));
    connect(udisks, SIGNAL(blockDeviceRemoved(QString)), this, SLOT(blockDevicesChanged()));
    connect(udisks, SIGNAL(filesystemAdded(QString)), this, SLOT(blockDevicesChanged()));

    blockDevicesChanged();

    currentDir = QDir::home();
    reloadList();

}

void MainWindow::blockDevicesChanged() {
    ui->fav->clear();
    favDirs.clear();

    QFile fav(QDir::homePath() + "/.thefile/fav");
    fav.open(QFile::ReadOnly);
    QString favData(fav.readAll());
    fav.close();
    for (QString line : favData.split("\n")) {
        if (line == "") {
            continue;
        }
        QStringList dat = line.split(",");
        QListWidgetItem *item = new QListWidgetItem(dat.at(1));
        if (QDir::homePath() == dat.at(0)) {
            item->setIcon(QIcon::fromTheme("user-home"));
        } else {
            item->setIcon(QIcon::fromTheme("folder"));
        }
        ui->fav->addItem(item);
        favDirs.append(dat.at(0));
    }

    QProcess *lsblk = new QProcess(this);
    lsblk->start("lsblk -rf --output name,label,hotplug");

    lsblk->waitForFinished();
    QByteArray output = lsblk->readAllStandardOutput();

    for (QString block : udisks->blockDevices()) {
        try {
        UDisks2Block *device = udisks->blockDevice(block);
        QListWidgetItem *item;
        QIcon icon;
        if (device->fileSystem()) {
            if (device->type != "swap") {
            for (QString part : QString(output).split("\n")) {
                if (part != "") {
                    QStringList parse = part.split(" ");
                    if (parse.length() > 1) {
                        if (parse[0] == device->fileSystem()->name) {
                            if (parse[1] == "") {
                                item = new QListWidgetItem(calculateSize(device->size) + " Hard Drive (" + device->fileSystem()->name + ")");
                                icon = QIcon::fromTheme("drive-harddisk");
                            } else {

                                if (parse.count() > 2) {
                                    if (parse[2] == "0") {
                                        icon = QIcon::fromTheme("drive-harddisk");
                                    } else {
                                        icon = QIcon::fromTheme("drive-removable-media");
                                    }
                                }





                                QString itemText(parse[1].replace("\\x20", " ") + " (" + device->fileSystem()->name + ")");
                                item = new QListWidgetItem(itemText);
                            }
                        }
                    }
                }
            }

            if (!(item)) {
                item = new QListWidgetItem(calculateSize(device->size) + " Hard Drive (" + device->fileSystem()->name + ")");
                icon = QIcon::fromTheme("drive-harddisk");
            }

            if (device->fileSystem()->mountPoints().count() == 0) {
                QPainter *p = new QPainter();
                QPixmap temp = icon.pixmap(16,16);
                p->begin(&temp);
                p->drawPixmap(8,8,8,8,QIcon::fromTheme("emblem-unmounted").pixmap(8,8));
                p->end();
                icon = QIcon(temp);
            } else {
                QPainter *p = new QPainter();
                QPixmap temp = icon.pixmap(16,16);
                p->begin(&temp);
                p->drawPixmap(8,8,8,8,QIcon::fromTheme("emblem-mounted").pixmap(8,8));
                p->end();
                icon = QIcon(temp);
            }


            item->setIcon(icon);
            ui->fav->addItem(item);

        }
        }
        } catch (QException e) {

        } catch (...) {

        }
    }
}


MainWindow::~MainWindow()
{
    delete udisks;
    delete ui;
}

void MainWindow::reloadList() {
    ui->errorLabel->setVisible(false);
    watcher->removePaths(watcher->directories());
    watcher->addPath(currentDir.path());
    currentDir.refresh();

    ui->files->clearContents();
    //QList<QTableWidgetItem*> items();
    //QDirIterator iterator(currentDir, QDirIterator::NoIteratorFlags);
    if (!currentDir.exists()) {
        //ui->errorLabel->setVisible(true);
        //ui->errorLabel->setText("This folder doesn't exist.");
        //ui->files->setRowCount(0);
        ui->message->setText("This folder doesn't exist.");
        ui->message->setMessageType(KMessageWidget::Error);
        ui->message->setCloseButtonVisible(false);
        ui->message->animatedShow();
        ui->files->setRowCount(0);
    } else if (currentDir.entryList().count() == 2) {
        //ui->errorLabel->setVisible(true);
        //ui->errorLabel->setText("There aren't any files to see here.");
        //ui->files->setRowCount(0);
        ui->message->setText("There aren't any files to see here.");
        ui->message->setMessageType(KMessageWidget::Warning);
        ui->message->setCloseButtonVisible(false);
        ui->message->animatedShow();
        ui->files->setRowCount(0);
    } else if (!currentDir.isReadable()) {
        ui->message->setText("Can't read this folder.");
        ui->message->setMessageType(KMessageWidget::Error);
        ui->message->setCloseButtonVisible(false);
        ui->message->animatedShow();
        ui->files->setRowCount(0);
    } else {
        ui->message->animatedHide();
        int i = 0;
        ui->files->setRowCount(currentDir.count() - 2);

        for (QString file : currentDir.entryList()) { //Get Directories
            if (file == "." || file == "..") {
                continue;
            }
            if (QDir(currentDir.path() + "/" + file).exists()) {
                QTableWidgetItem *name = new QTableWidgetItem(file);
                name->setIcon(QIcon::fromTheme("folder"));
                ui->files->setItem(i, 0, name);

                QTableWidgetItem *type = new QTableWidgetItem("Folder");
                ui->files->setItem(i, 1, type);
                i++;

            }
        }
        for (QString file : currentDir.entryList()) { //Get Files
            if (file == "." || file == "..") {
                continue;
            }
            if (!QDir(currentDir.path() + "/" + file).exists()) {
                QFile f(currentDir.path() + "/" + file);
                QFileInfo info(f);
                QTableWidgetItem *name = new QTableWidgetItem(file);
                name->setIcon(icons->icon(QFileIconProvider::File));

                ui->files->setItem(i, 0, name);


                QTableWidgetItem *type = new QTableWidgetItem(info.completeSuffix() + " file");
                ui->files->setItem(i, 1, type);

                QTableWidgetItem *size = new QTableWidgetItem(calculateSize(info.size()));
                ui->files->setItem(i, 2, size);

                QTableWidgetItem *date = new QTableWidgetItem(info.lastModified().toString("dd/MM/yy hh:mm"));
                ui->files->setItem(i, 3, date);
                i++;
            }
        }
    }

    /*while (iterator.hasNext()) {
        QString file = iterator.next();
        if (QDir(file).exists()) {
            QDir dir(file);
            QTableWidgetItem *name = new QTableWidgetItem(dir.currentPath().remove(currentDir.absolutePath()));
            ui->files->setItem(i, 0, name);

        } else {
            QFile f(file);
            QTableWidgetItem *name = new QTableWidgetItem(f.fileName());
            ui->files->setItem(i, 0, name);
        }
        i++;
    }*/

    ui->addr->setText(currentDir.path());
    this->setWindowTitle(currentDir.path() + " - theFile");

    bool showUnmount = false;
    for (QString device : udisks->blockDevices()) {
        if (udisks->blockDevice(device)->fileSystem()) {
            if (udisks->blockDevice(device)->fileSystem()->mountPoints().contains(currentDir.path())) {
                showUnmount = true;
            }
        }
    }

    ui->actionUnmount->setVisible(showUnmount);
    blockDevicesChanged();
}

void MainWindow::on_files_itemDoubleClicked(QTableWidgetItem *item)
{
    if (currentDir.cd(ui->files->item(item->row(), 0)->text())) {
        reloadList();
    } else {
        if (QFile(currentDir.path() + item->text()).permissions() & QFile::ExeOther) {
            QProcess::execute(currentDir.path() + item->text());
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir.path() + "/" + item->text()));
        }
    }
}

void MainWindow::on_actionUp_triggered()
{
    currentDir.cdUp();
    reloadList();
}

void MainWindow::on_fav_itemDoubleClicked(QListWidgetItem *item)
{

}

void MainWindow::on_actionAbout_triggered()
{
    about *a = new about(this);
    a->exec();
}

void MainWindow::on_files_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cx = new QMenu(this);
    if (ui->files->selectedItems().count() != 0) {
        cx->addSection("For \"" + ui->files->selectedItems().at(0)->text() + "\"");
        cx->addAction(ui->actionOpen);
        cx->addAction(ui->actionRename);
        if (ui->files->selectedItems().count() > 1) {
            cx->addSection("For all selected objects");
        }
        cx->addAction(ui->actionCopy);
        cx->addAction(ui->actionDelete);
    }

    cx->addSection("For folder \"" + currentDir.dirName() + "\"");
    cx->addAction(ui->actionPaste);
    cx->addAction(ui->actionNew_Folder);

    cx->exec(ui->files->mapToGlobal(pos));
}

void MainWindow::on_actionOpen_triggered()
{
    if (ui->files->selectedItems().count() > 0) {
        on_files_itemDoubleClicked(ui->files->selectedItems().at(0));
    }
}

void MainWindow::on_actionCopy_triggered()
{
    QMimeData *d = new QMimeData();
    QList<QUrl> allFiles;

    for (QTableWidgetItem *item : ui->files->selectedItems()) {
        if (item->column() == 0) {
            allFiles.append(QUrl::fromLocalFile(currentDir.path() + "/" + item->text()));
        }
    }

    d->setUrls(allFiles);

    QApplication::clipboard()->setMimeData(d);
}

void MainWindow::on_actionDelete_triggered()
{
    if (QMessageBox::question(this, "Delete File", "You're about to permanantly delete files. These files can't be recovered.", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        for (QTableWidgetItem *item : ui->files->selectedItems()) {
            if (item->column() == 0) {
                if (!QFile(currentDir.path() + "/" + item->text()).remove()) {
                    //currentDir.rmdir(item->text());
                    QDir dirToRemove(currentDir);
                    dirToRemove.cd(item->text());
                    dirToRemove.removeRecursively();
                }
            }
        }
    }

    reloadList();
}

void MainWindow::on_actionPaste_triggered()
{
    if (QApplication::clipboard()->mimeData()->hasUrls()) {
        QStringList *files = new QStringList();
        for (QUrl urlFile : QApplication::clipboard()->mimeData()->urls()) {
            if (urlFile.scheme() == "file") {
                files->append(urlFile.path());
            }
        }

        //const copy *copyOp = //, this);
        copyops.append(new copy(files, currentDir.path() + "/"));
    }
}

void MainWindow::on_files_entered(const QModelIndex &index)
{

}

void MainWindow::on_actionRename_triggered()
{
    ui->files->editItem(ui->files->selectedItems().at(0));
    oldFileName = ui->files->selectedItems().at(0)->text();
}

void MainWindow::on_files_itemChanged(QTableWidgetItem *item)
{
    if (oldFileName != "") {
        if (oldFileName == "mkdir") {
            currentDir.mkdir(item->text());
        } else {

            QFile file(currentDir.path() + "/" + oldFileName);
            file.rename(currentDir.path() + "/" + item->text());
        }
        oldFileName = "";
    }
    //reloadList();
}

void MainWindow::on_actionNew_Folder_triggered()
{
    ui->files->setRowCount(ui->files->rowCount() + 1);

    QTableWidgetItem *newItem = new QTableWidgetItem();
    newItem->setText("New Folder");
    newItem->setIcon(QIcon::fromTheme("folder"));
    ui->files->setItem(ui->files->rowCount() - 1, 0, newItem);
    oldFileName = "mkdir";
    ui->files->editItem(newItem);
}

void MainWindow::on_actionAdd_To_Bar_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add to Bar", "What do you want to name this folder?", QLineEdit::Normal, currentDir.dirName(), &ok);
    if (ok) {
        QFile fav(QDir::homePath() + "/.thefile/fav");
        fav.open(QFile::Append);
        fav.write(QString(currentDir.path() + "," + text + "\n").toUtf8());
        fav.close();
        blockDevicesChanged();
    }
}

void MainWindow::on_addr_returnPressed()
{
    //if (QDir(ui->addr->text()).exists()) {
        currentDir.setPath(ui->addr->text());
        reloadList();
    /*} else {
        QMessageBox::warning(this, "Nonexistent directory", "That folder doesn't exist.", QMessageBox::Ok, QMessageBox::Ok);
    }*/
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->files->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = (QMouseEvent*) event;
            if (mouseEvent->button() == Qt::LeftButton) {
                startPos = mouseEvent->pos();
            }
            if (ui->files->selectedItems().count() > 1) {
                mouseEvent->ignore();
                return false;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = (QMouseEvent*) event;
            if (mouseEvent->buttons() & Qt::LeftButton) {
                if ((mouseEvent->pos() - startPos).manhattanLength() > QApplication::startDragDistance()) {
                    if (ui->files->selectedItems().count() > 0) {
                        QDrag *drag = new QDrag(this);
                        QMimeData *mime = new QMimeData;
                        QList<QUrl> allFiles;

                        for (QTableWidgetItem *item : ui->files->selectedItems()) {
                            if (item->column() == 0) {
                                allFiles.append(QUrl::fromLocalFile(currentDir.path() + "/" + item->text()));
                            }
                        }

                        mime->setUrls(allFiles);
                        drag->setMimeData(mime);
                        drag->exec(Qt::CopyAction);
                    }
                }
            }
            return false;
        }
    } else if (obj == ui->files) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *mouseEvent = (QDragEnterEvent*) event;
            if (mouseEvent->mimeData() != NULL) {
                mouseEvent->acceptProposedAction();
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *mouseEvent = (QDropEvent*) event;
            if (mouseEvent->mimeData()->hasUrls()) {
                QStringList *files = new QStringList();
                for (QUrl urlFile : mouseEvent->mimeData()->urls()) {
                    if (urlFile.scheme() == "file") {
                        files->append(urlFile.path());
                    }
                }

                copyops.append(new copy(files, currentDir.path() + "/"));
            }
        }
    }
    return false;
}

void MainWindow::on_fav_itemClicked(QListWidgetItem *item)
{
    if (favDirs.count() - 1 < ui->fav->selectionModel()->selectedIndexes().at(0).row()) {
        QString dev = item->text().mid(item->text().indexOf("(") + 1, item->text().indexOf(")") - item->text().indexOf("(") - 1);
        qDebug() << "Mounting " + dev;
        if (udisks->blockDevice(dev)->fileSystem()->mountPoints().count() == 0) {
            QString mountpoint = udisks->blockDevice(dev)->fileSystem()->mount();
            while (udisks->blockDevice(dev)->fileSystem()->mountPoints().count() == 0) {
                QApplication::processEvents();
            }
            if (mountpoint == "") {
                ui->messageFav->setText("Couldn't mount " + udisks->blockDevice(dev)->dev);
                ui->messageFav->animatedShow();
            } else {
                currentDir.setPath(mountpoint);
            }
        } else {
            currentDir.setPath(udisks->blockDevice(dev)->fileSystem()->mountPoints().at(0));
        }
        reloadList();
        blockDevicesChanged();
    } else {
        currentDir.setPath(favDirs.at(ui->fav->selectionModel()->selectedIndexes().at(0).row()));
        reloadList();
    }
}

void MainWindow::on_actionUnmount_triggered()
{
    for (QString device : udisks->blockDevices()) {
        if (udisks->blockDevice(device)->fileSystem()) {
            if (udisks->blockDevice(device)->fileSystem()->mountPoints().contains(currentDir.path())) {
                qDebug() << "Unmounting " + udisks->blockDevice(device)->name;
                udisks->blockDevice(device)->fileSystem()->unmount();
                currentDir.setPath(QDir::homePath());
                while (udisks->blockDevice(device)->fileSystem()->mountPoints().count() != 0) {
                    QApplication::processEvents();
                }

                ui->message->setText("The device was unmounted successfully.");
                ui->message->setMessageType(KMessageWidget::Positive);
                ui->message->setCloseButtonVisible(true);
                ui->message->animatedShow();
                blockDevicesChanged();
            }
        }
    }
}

void MainWindow::on_fav_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cx = new QMenu(this);

    if (ui->fav->selectedItems().count() != 0) {
        cx->addSection("For \"" + ui->fav->selectedItems().at(0)->text() + "\"");
        cx->addAction(ui->actionUnmount2);
    }

    cx->exec(ui->fav->mapToGlobal(pos));
}

void MainWindow::on_actionUnmount2_triggered()
{
    QString dev = ui->fav->selectedItems().at(0)->text().mid(ui->fav->selectedItems().at(0)->text().indexOf("(") + 1, ui->fav->selectedItems().at(0)->text().indexOf(")") - ui->fav->selectedItems().at(0)->text().indexOf("(") - 1);
    for (QString device : udisks->blockDevices()) {
        if (udisks->blockDevice(device)->fileSystem()) {
            if (udisks->blockDevice(device)->name == dev) {
                qDebug() << "Unmounting " + dev;
                udisks->blockDevice(device)->fileSystem()->unmount();
                while (udisks->blockDevice(device)->fileSystem()->mountPoints().count() != 0) {
                    QApplication::processEvents();
                }

                ui->message->setText("The device was unmounted successfully.");
                ui->message->setMessageType(KMessageWidget::Positive);
                ui->message->setCloseButtonVisible(true);
                ui->message->animatedShow();
                blockDevicesChanged();
            }
        }
    }
}
