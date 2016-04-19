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
    ui->movedToTrashFrame->setVisible(false);
    ui->deviceContentFrame->setVisible(false);

    if (copyops.count() == 0) {
        copyops = QList<copy*>();
    }

    if (transferWin == 0) {
        transferWin = new fileTransfers();
    }

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadList()));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadList()));

    icons = new QFileIconProvider();
    mimes = new QMimeDatabase();

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
        fav.write(QString(QDir::homePath() + ",Home\n/,Root").toUtf8());
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

    //Add user's favourites
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
        } else if (dat.at(0) == "/") {
            item->setIcon(QIcon::fromTheme("computer"));
        } else {
            item->setIcon(QIcon::fromTheme("folder"));
        }
        ui->fav->addItem(item);
        favDirs.append(dat.at(0));
    }

    QListWidgetItem* sep1 = new QListWidgetItem();
    sep1->setSizeHint(QSize(50, 1));
    sep1->setFlags(Qt::NoItemFlags);
    ui->fav->addItem(sep1);

    QFrame *sepLine1 = new QFrame();
    sepLine1->setFrameShape(QFrame::HLine);
    ui->fav->setItemWidget(sep1, sepLine1);

    //Add important directories
    QListWidgetItem *trash = new QListWidgetItem("Trash");
    trash->setIcon(QIcon::fromTheme("user-trash"));
    ui->fav->addItem(trash);

    QListWidgetItem* sep2 = new QListWidgetItem();
    sep2->setSizeHint(QSize(50, 1));
    sep2->setFlags(Qt::NoItemFlags);
    ui->fav->addItem(sep2);

    QFrame *sepLine2 = new QFrame();
    sepLine2->setFrameShape(QFrame::HLine);
    ui->fav->setItemWidget(sep2, sepLine2);

    //Add detected block devices
    QProcess *lsblk = new QProcess(this);
    lsblk->start("lsblk -rf --output name,label,hotplug,parttype");

    lsblk->waitForFinished();
    QByteArray output = lsblk->readAllStandardOutput();

    for (QString block : udisks->blockDevices()) { //Iterate over all detected block devices
        UDisks2Block *device = udisks->blockDevice(block);
        QListWidgetItem *item;
        QIcon icon;
        if (device) { //Check that device actually exists
            if (device->fileSystem()) { //Check that filesystem exists on block device
                if (device->type != "swap") { //Ignore swap devices
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
                    item->setData(Qt::UserRole, device->fileSystem()->name);
                    ui->fav->addItem(item);
                }
            }
        }
    }

    QListWidgetItem* sep3 = new QListWidgetItem();
    sep3->setSizeHint(QSize(50, 1));
    sep3->setFlags(Qt::NoItemFlags);
    ui->fav->addItem(sep3);

    QFrame *sepLine3 = new QFrame();
    sepLine3->setFrameShape(QFrame::HLine);
    bool hasItem = false;
    ui->fav->setItemWidget(sep3, sepLine3);

    if (QFile("/usr/bin/jmtpfs").exists()) {
        //Detect MTP devices
        QProcess* mtpDev = new QProcess(this);
        mtpDev->start("jmtpfs -l");
        mtpDev->waitForStarted();


        while (mtpDev->state() == QProcess::Running) {
            QApplication::processEvents();
        }
        QString output(mtpDev->readAll());
        bool startReading = false;
        for (QString line : output.split("\n")) {
            if (line != "") {
                if (startReading) {
                    hasItem = true;
                    QStringList parse = line.split(", "); //busLocation, devNum, productId, vendorId, product, vendor
                    QListWidgetItem* item = new QListWidgetItem();
                    QString text = parse.at(5) + " " + parse.at(4);
                    if (!text.contains("(MTP)")) {
                        text += " (MTP)";
                    }
                    item->setText(parse.at(5) + " " + parse.at(4));
                    item->setIcon(QIcon::fromTheme("smartphone"));
                    item->setData(Qt::UserRole, "mtp");
                    item->setData(Qt::UserRole + 1, parse.at(0));
                    item->setData(Qt::UserRole + 2, parse.at(1));
                    ui->fav->addItem(item);
                } else {
                    if (line.startsWith("Available devices")) {
                        startReading = true;
                    }
                }
            }
        }

    }

    if (QFile("/usr/bin/ifuse").exists() && QFile("/usr/bin/idevicepair").exists() && QFile("/usr/bin/idevice_id").exists()) {
        //Detect iOS Devices

        QProcess* iosDev = new QProcess();
        iosDev->start("idevice_id -l");
        iosDev->waitForStarted();

        while (iosDev->state() == QProcess::Running) {
            QApplication::processEvents();
        }

        QString output(iosDev->readAll());
        for (QString line : output.split("\n")) {
            if (line != "") {
                if (!line.startsWith("ERROR:")) {
                    QListWidgetItem* item = new QListWidgetItem();
                    QProcess* iosName = new QProcess();
                    iosName->start("idevice_id " + line);
                    iosName->waitForFinished();

                    QString name(iosName->readAll());
                    name = name.trimmed();

                    if (name == "") {
                        item->setText("iOS Device");
                    } else {
                        item->setText(name + " (iOS)");
                    }

                    item->setIcon(QIcon::fromTheme("smartphone"));
                    item->setData(Qt::UserRole, "ios");
                    item->setData(Qt::UserRole + 1, line);
                    ui->fav->addItem(item);
                    hasItem = true;
                }
            }
        }
    }

    if (!hasItem) {
        delete sepLine3;
        delete sep3;
    }
}


MainWindow::~MainWindow()
{
    delete udisks;
    delete ui;
}

void MainWindow::reloadList() {
    if (ui->files->isEnabled()) {
        ui->files->setEnabled(false);
        watcher->removePaths(watcher->directories());
        watcher->addPath(currentDir.path());
        currentDir.refresh();

        ui->files->clearContents();
        if (!currentDir.exists()) {
            ui->message->setText("This folder doesn't exist.");
            ui->message->setMessageType(KMessageWidget::Error);
            ui->message->setCloseButtonVisible(false);
            ui->message->animatedShow();
            ui->files->setRowCount(0);
        } else if (currentDir.entryList().count() == 2) {
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
            //ui->files->setRowCount(currentDir.count() - 2);
            ui->files->setRowCount(0);

            for (QString file : currentDir.entryList()) { //Get Directories
                if (file == "." || file == "..") {
                    continue;
                }
                if (QDir(currentDir.path() + "/" + file).exists()) {
                    ui->files->setRowCount(ui->files->rowCount() + 1);
                    QTableWidgetItem *name = new QTableWidgetItem(file);

                    //Give special directories special icons
                    if (currentDir.path() + "/" + file == QDir::homePath()) { //Home Folder
                        name->setIcon(QIcon::fromTheme("user-home"));
                    } else if (currentDir.path() + "/" + file == QDir::homePath() + "/Documents") { //Documents Folder
                        name->setIcon(QIcon::fromTheme("folder-documents"));
                    } else if (currentDir.path() + "/" + file == QDir::homePath() + "/Downloads") { //Downloads Folder
                        name->setIcon(QIcon::fromTheme("folder-downloads"));
                    } else if (currentDir.path() + "/" + file == QDir::homePath() + "/Music") { //Music Folder
                        name->setIcon(QIcon::fromTheme("folder-music"));
                    } else if (currentDir.path() + "/" + file == QDir::homePath() + "/Pictures") { //Pictures Folder
                        name->setIcon(QIcon::fromTheme("folder-pictures"));
                    } else if (currentDir.path() + "/" + file == QDir::homePath() + "/Videos") { //Videos Folder
                        name->setIcon(QIcon::fromTheme("folder-videos"));
                    } else {
                        if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp") &&
                                !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length()).contains("/")) { //MTP Root Folder
                            if (currentDir.path() + "/" + file == currentDir.path() + "/Card") { //Memory Card
                                name->setIcon(QIcon::fromTheme("drive-removable-media"));
                            } else if (currentDir.path() + "/" + file == currentDir.path() + "/Phone" || currentDir.path() + "/" + file == currentDir.path() + "/Internal storage") { //Internal Storage
                                name->setIcon(QIcon::fromTheme("smartphone"));
                            } else { //Generic Folder
                                name->setIcon(QIcon::fromTheme("folder"));
                            }
                        } else { //Generic Folder
                            name->setIcon(QIcon::fromTheme("folder"));
                        }
                    }
                    ui->files->setItem(i, 0, name);

                    QTableWidgetItem *type = new QTableWidgetItem("Folder");
                    ui->files->setItem(i, 1, type);

                    if (file.startsWith(".")) {
                        QBrush disabledColor = ui->files->palette().brush(QPalette::Disabled, QPalette::Foreground);
                        name->setForeground(disabledColor);
                        type->setForeground(disabledColor);
                    }
                    i++;

                }
                QApplication::processEvents();
            }
            for (QString file : currentDir.entryList()) { //Get Files
                if (file == "." || file == "..") {
                    continue;
                }
                if (!QDir(currentDir.path() + "/" + file).exists()) {
                    ui->files->setRowCount(ui->files->rowCount() + 1);
                    QFile f(currentDir.path() + "/" + file);
                    QFileInfo info(f);
                    QTableWidgetItem *name = new QTableWidgetItem(file);
                    QMimeType mime = mimes->mimeTypeForFile(info);
                    name->setIcon(QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("application-octet-stream")));

                    ui->files->setItem(i, 0, name);

                    QTableWidgetItem *type = new QTableWidgetItem(info.completeSuffix() + " file");
                    ui->files->setItem(i, 1, type);

                    QTableWidgetItem *size = new QTableWidgetItem(calculateSize(info.size()));
                    ui->files->setItem(i, 2, size);

                    QTableWidgetItem *date = new QTableWidgetItem(info.lastModified().toString("dd/MM/yy hh:mm"));
                    ui->files->setItem(i, 3, date);

                    if (file.startsWith(".")) {
                        QBrush disabledColor = ui->files->palette().brush(QPalette::Disabled, QPalette::Foreground);
                        name->setForeground(disabledColor);
                        type->setForeground(disabledColor);
                        size->setForeground(disabledColor);
                        date->setForeground(disabledColor);
                    }

                    i++;
                }
                QApplication::processEvents();
            }
        }

        if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files") { //This is the trash.
            ui->addr->setText("Trash");
            this->setWindowTitle("Trash - theFile");
        } else if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp")) { //This is an MTP device
            QString titleText = "mtp:" + currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length());
            ui->addr->setText(titleText);
            this->setWindowTitle(titleText + " - theFile");
        } else if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios")) { //This is an iOS Device
            QString titleText = "ios:" + currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length());
            ui->addr->setText(titleText);
            this->setWindowTitle(titleText + " - theFile");
        } else { //This is not an interesting folder, show path in address bar
            ui->addr->setText(currentDir.path());
            this->setWindowTitle(currentDir.path() + " - theFile");
        }

        if (currentDir.path() == QDir::rootPath()) {
            ui->actionUnmount->setVisible(false);
            ui->deviceContentFrame->setVisible(false);
        } else {
            bool showUnmount = false;
            if ((currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp") &&
                    !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length()).contains("/")) ||
                    (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios") &&
                    !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/ios").length()).contains("/"))) {
                showUnmount = true;
            } else {
                for (QString device : udisks->blockDevices()) {
                    if (udisks->blockDevice(device)->fileSystem()) {
                        if (udisks->blockDevice(device)->fileSystem()->mountPoints().contains(currentDir.path())) {
                            showUnmount = true;
                        }
                    }
                }
            }

            if (showUnmount) {
                if (QDir(currentDir.path() + "/DCIM").exists()) {
                    ui->deviceContentFrame->setVisible(true);
                } else {
                    ui->deviceContentFrame->setVisible(false);
                }
            } else {
                ui->deviceContentFrame->setVisible(false);
            }
            ui->actionUnmount->setVisible(showUnmount);
        }

        blockDevicesChanged();
        ui->files->setEnabled(true);
    }
}

void MainWindow::on_files_itemDoubleClicked(QTableWidgetItem *item)
{
    if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files") { //File is in trash. Don't allow opening.
        QMessageBox::critical(this, "File in trash", "This file/folder is in the trash. Restore the file to open it.", QMessageBox::Ok, QMessageBox::Ok);
    } else {
        if (currentDir.cd(ui->files->item(item->row(), 0)->text())) {
            reloadList();
        } else {
            QFile file(currentDir.path() + "/" + ui->files->item(item->row(), 0)->text());

            if (file.permissions() & QFile::ExeUser) {
                    if (!QProcess::startDetached("\"" + currentDir.path() + "/" + ui->files->item(item->row(), 0)->text() + "\"")) {
                        QProcess::startDetached("xdg-open", QStringList() << currentDir.path() + "/" + ui->files->item(item->row(), 0)->text());
                    }
            } else {
                //QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir.path() + "/" + item->text()));
                QProcess::startDetached("xdg-open", QStringList() << currentDir.path() + "/" + ui->files->item(item->row(), 0)->text());
            }
        }
    }
}

void MainWindow::on_actionUp_triggered()
{
    if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files" ||
            (currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp") &&
             !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length()).contains("/")) ||
            (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios") &&
            !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/ios").length()).contains("/"))) { //This is trash, MTP root or iOS, go to home instead
        currentDir.cd(QDir::homePath());
        reloadList();
    } else { //Go up a folder
        currentDir.cdUp();
        reloadList();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    about *a = new about(this);
    a->exec();
}

void MainWindow::on_files_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cx = new QMenu(this);

    //Detect whether this is the Trash folder because menus are different
    if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files") { //This is the Trash folder
        if (ui->files->selectedItems().count() != 0) {
            cx->addSection("For all selected objects");
            cx->addAction(ui->actionRestore);
            cx->addAction(ui->actionDelete);
        }
        cx->addSection("For all items in Trash");
        cx->addAction(ui->actionEmpty_Trash);
    } else {
        if (ui->files->selectedItems().count() != 0) {
            cx->addSection("For \"" + ui->files->selectedItems().at(0)->text() + "\"");
            cx->addAction(ui->actionOpen);
            cx->addAction(ui->actionRename);
            cx->addAction(ui->actionProperties);
            if (ui->files->selectedItems().count() > 1) {
                cx->addSection("For all selected objects");
            }
            cx->addAction(ui->actionCopy);
            if (!currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp")) { //Detect if this is MTP device
                cx->addAction(ui->actionMove_to_Trash);
            }
            cx->addAction(ui->actionDelete);
        }

        cx->addSection("For folder \"" + currentDir.dirName() + "\"");
        cx->addAction(ui->actionPaste);
        cx->addAction(ui->actionNew_Folder);
    }

    cx->exec(ui->files->mapToGlobal(pos));
}

void MainWindow::on_actionOpen_triggered()
{
    if (ui->addr->hasFocus()) {
        on_addr_returnPressed();
    } else if (ui->files->selectedItems().count() > 0) {
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

                if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files") { //This file is in trash, also remove metadata
                    QDir trash = QDir::home();
                    trash.cd(".local/share/Trash");
                    QFile info(trash.path() + "/info/" + item->text() + ".trashinfo");
                    info.remove();
                }
            }
        }
    }

    reloadList();
}

void MainWindow::on_actionPaste_triggered()
{
    if (QApplication::clipboard()->mimeData()->hasUrls()) {
        QStringList files;
        for (QUrl urlFile : QApplication::clipboard()->mimeData()->urls()) {
            if (urlFile.scheme() == "file") {
                files.append(urlFile.path());
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
            if (QFileInfo(currentDir.path() + "/" + oldFileName).suffix() != QFileInfo(currentDir.path() + "/" + item->text()).suffix()) {
                if (QMessageBox::warning(this, "Changing File Extension", "Changing the file extension may cause "
                                         "the file to not open in the correct application. Do you want to do this anyway?",
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
                    item->setText(oldFileName);
                    return;
                }
            }

            if (QFile(currentDir.path() + "/" + item->text()).exists() || QDir(currentDir.path() + "/" + item->text()).exists()) {
                ui->message->setText("There's already a file/folder named " + item->text() + " in this folder. Do something with that first!");
                ui->message->animatedShow();
                item->setText(oldFileName);
                return;
            }

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
    currentDir.setPath(ui->addr->text());
    reloadList();
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
                QStringList files;
                for (QUrl urlFile : mouseEvent->mimeData()->urls()) {
                    if (urlFile.scheme() == "file") {
                        files.append(urlFile.path());
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
    //Decide if clicked item is a favourite or not
    if (favDirs.count() - 1 < ui->fav->selectionModel()->selectedIndexes().at(0).row()) { //The item is not a favourite
        //Determine whether this is an interesting folder or a device
        if (item->text() == "Trash") { //This is the trash folder; navigate to the trash folder
            currentDir.setPath(QDir::homePath() + "/.local/share/Trash/files");
            reloadList();
        } else { //This is a block or MTP device; mount and navigate to device.
            QString dev = item->data(Qt::UserRole).toString();
            if (dev == "mtp") {
                qDebug() << "Mounting MTP device " + item->data(Qt::UserRole + 1).toString() + ", " + item->data(Qt::UserRole + 2).toString();

                QString mtpDirName = "mtp" + item->data(Qt::UserRole + 1).toString() + "," + item->data(Qt::UserRole + 2).toString();
                QDir::home().mkdir(".thefile");
                QDir(QDir::homePath() + "/.thefile").mkdir(mtpDirName);
                QProcess *mountProcess = new QProcess(this);

                bool mounted = false;
                for (QString file : QDir(QDir::homePath() + "/.thefile/" + mtpDirName).entryList()) {
                    if (file != "." && file != "..") {
                        mounted = true;
                    }
                }
                if (!mounted) {
                    mountProcess->start("jmtpfs " + QDir::homePath() + "/.thefile/" + mtpDirName + " -device=" + item->data(Qt::UserRole + 1).toString() + "," + item->data(Qt::UserRole + 2).toString());
                    mountProcess->waitForStarted();

                    while (mountProcess->state() == QProcess::Running) {
                        QApplication::processEvents();
                    }
                }

                currentDir.setPath(QDir::homePath() + "/.thefile/" + mtpDirName);
            } else if (dev == "ios") { //iOS Device
                QString id = item->data(Qt::UserRole + 1).toString();
                qDebug() << "Mounting iOS Device " + id;

                QProcess* pairProcess = new QProcess(this);
                pairProcess->start("idevicepair -u " + id + " pair");
                pairProcess->waitForStarted();

                while (pairProcess->state() == QProcess::Running) {
                    QApplication::processEvents();
                }

                QString pairOutput(pairProcess->readAll());
                if (pairOutput.startsWith("ERROR:")) {
                    if (pairOutput.contains("Please enter the passcode")) { // Ask user to unlock device
                        QMessageBox::critical(this, "iOS Device Locked", "The device is locked. Enter the passcode on the device and try again.", QMessageBox::Ok, QMessageBox::Ok);
                    } else if (pairOutput.contains("Please accept the trust dialog")) { //Ask user to trust PC
                        QMessageBox::critical(this, "iOS Device Trust", "Your device does not trust this PC. To access the device, you need to trust this PC. Answer the trust dialog on your device and try again.", QMessageBox::Ok, QMessageBox::Ok);
                    } else if (pairOutput.contains("user denied the trust dialog")) { //User did not trust PC
                        QMessageBox::critical(this, "iOS Device Trust", "We can't access this device because you told it to not trust this computer.", QMessageBox::Ok, QMessageBox::Ok);
                    } else { //Generic Error
                        QMessageBox::critical(this, "iOS Error", "An error occurred trying to pair with the device:\n\n" + pairOutput, QMessageBox::Ok, QMessageBox::Ok);
                    }
                    return;
                } else {
                    QString iosDirName = "ios" + id;
                    QDir::home().mkdir(".thefile");
                    QDir(QDir::homePath() + "/.thefile").mkdir(iosDirName);

                    QProcess* mountProcess = new QProcess();

                    bool mounted = false;
                    for (QString file : QDir(QDir::homePath() + "/.thefile/" + iosDirName).entryList()) {
                        if (file != "." && file != "..") {
                            mounted = true;
                        }
                    }
                    if (!mounted) {
                        mountProcess->start("ifuse -o ro " + QDir::homePath() + "/.thefile/" + iosDirName + " -u " + id);
                        mountProcess->waitForStarted();

                        while (mountProcess->state() == QProcess::Running) {
                            QApplication::processEvents();
                        }
                    }

                    currentDir.setPath(QDir::homePath() + "/.thefile/" + iosDirName);
                }

            } else {
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
            }
            reloadList();
            blockDevicesChanged();
        }
    } else { //The item is a favourite; navigate to that location
        currentDir.setPath(favDirs.at(ui->fav->selectionModel()->selectedIndexes().at(0).row()));
        reloadList();
    }
}

void MainWindow::on_actionUnmount_triggered()
{
    if ((currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp") &&
            !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length()).contains("/")) ||
            (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios") &&
            !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/ios").length()).contains("/"))) {
        QProcess::execute("fusermount -u " + currentDir.path());
        currentDir.removeRecursively();
        currentDir.setPath(QDir::homePath());

        ui->message->setText("The device was unmounted successfully.");
        ui->message->setMessageType(KMessageWidget::Positive);
        ui->message->setCloseButtonVisible(true);
        ui->message->animatedShow();
    } else {
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
                }
            }
        }
    }
}

void MainWindow::on_fav_customContextMenuRequested(const QPoint &pos)
{
    if (ui->fav->selectedItems().count() != 0) {
        QMenu *cx = new QMenu(this);
            if (favDirs.count() - 1 < ui->fav->selectionModel()->selectedIndexes().at(0).row()) {
                cx->addSection("For \"" + ui->fav->selectedItems().at(0)->text() + "\"");
                cx->addAction(ui->actionUnmount2);
            }
        cx->exec(ui->fav->mapToGlobal(pos));
    }
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

void MainWindow::goTo(QString dir) {
    currentDir.setPath(dir);
    reloadList();
}

void MainWindow::on_actionProperties_triggered()
{
    if (ui->files->selectedItems().count() > 0) {
        QFile *f = new QFile(currentDir.path() + "/" + ui->files->selectedItems().at(0)->text());
        Properties* p = new Properties(f, this);
        p->show();
    }
}

void MainWindow::on_fav_clicked(const QModelIndex &index)
{

}

void MainWindow::on_actionMove_to_Trash_triggered()
{
    filesMovedToTrash.clear();

    QDir trash = QDir::home();
    trash.cd(".local/share/");
    trash.mkdir("Trash");
    trash.cd("Trash");
    trash.mkdir("files");
    trash.mkdir("info");

    QFile directorySizes(trash.path() + "/directorysizes");
    directorySizes.open(QFile::Append);

    for (QTableWidgetItem *item : ui->files->selectedItems()) {
        if (item->column() == 0) {
            QString fileLocation = currentDir.path() + "/" + item->text();
            if (QFile(fileLocation).exists()) {
                //copyops.append(new copy(QStringList() << currentDir.path() + "/" + item->text(), trash.path() + "/files/", true));
                QFile(fileLocation).rename(trash.path() + "/files/" + item->text());
            } else {
                //QDir dirToRemove(currentDir);
                //dirToRemove.cd(item->text());
                currentDir.rename(item->text(), trash.path() + "/files/ " + item->text());
                //copyops.append(new copy(QStringList() << dirToRemove.path(), trash.path() + "/files/", true));
            }

            QFile trashinfo(trash.path() + "/info/" + item->text() + ".trashinfo");
            trashinfo.open(QFile::WriteOnly);
            trashinfo.write(QString("[Trash Info]\n").toUtf8());
            trashinfo.write(QString("Path=" + fileLocation + "\n").toUtf8());
            trashinfo.write(QString("DeletionDate=" + QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss") + "\n").toUtf8());
            trashinfo.close();

            ui->movedToTrashFrame->setVisible(true);
            filesMovedToTrash.append(item->text());

            QPropertyAnimation* a = new QPropertyAnimation(ui->movedToTrashFrameTimer, "value");
            connect(a, SIGNAL(finished()), this, SLOT(on_pushButton_2_clicked()));
            a->setStartValue(0);
            a->setEndValue(1000);
            a->setDuration(5000);
            a->start();
        }
    }

    reloadList();
}

void MainWindow::restoreFilesFromTrash(QStringList filenames) {
    QDir trash = QDir::home();
    trash.cd(".local/share/Trash");

    for (QString fileToRestore : filenames) {
        //Read Restore Info
        QFile info(trash.path() + "/info/" + fileToRestore + ".trashinfo");
        if (info.exists()) { //Sanity Check
            info.open(QFile::ReadWrite);
            QString moveTo;

            for (QString line : QString(info.readAll()).split("\n")) {
                if (line.startsWith("Path=")) {
                    moveTo = line.split("=").at(1); //Read original file path
                }
            }

            if (moveTo == "") { //Malformed info file

            } else {
                if (QFile(trash.path() + "/files/" + fileToRestore).exists()) { //Restore file
                    QFile(trash.path() + "/files/" + fileToRestore).rename(moveTo);
                } else if (QDir(trash.path() + "/files/" + fileToRestore).exists()) { //Restore Directory
                    QDir(trash.path() + "/files").rename(fileToRestore, moveTo);

                }
            }
            info.close();
            info.remove();
        }
    }
}

void MainWindow::on_actionNew_Window_triggered()
{
    //Create a new window and show it
    MainWindow* w = new MainWindow();
    w->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->movedToTrashFrame->setVisible(false);
}

void MainWindow::on_actionClose_Window_triggered()
{
    this->close();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit(0);
}

void MainWindow::on_pushButton_clicked()
{
    restoreFilesFromTrash(filesMovedToTrash);
    ui->movedToTrashFrame->setVisible(false);
}

void MainWindow::on_actionRestore_triggered()
{
    QStringList filesToRestore;
    for (QTableWidgetItem *item : ui->files->selectedItems()) {
        if (item->column() == 0) {
            filesToRestore.append(item->text());
        }
    }

    restoreFilesFromTrash(filesToRestore);

    reloadList();
}

void MainWindow::on_actionEmpty_Trash_triggered()
{
    ui->files->selectAll();
    on_actionDelete_triggered();
}

void MainWindow::on_actionShow_Hidden_Files_toggled(bool arg1)
{
    if (arg1) {
        currentDir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
    } else {
        currentDir.setFilter(QDir::Files | QDir::Dirs);
    }
    reloadList();
}

void MainWindow::on_openPhotoFolder_clicked()
{
    currentDir.cd("DCIM");
    reloadList();
}
