#include "mainwindow.h"
#include "ui_mainwindow.h"

extern QString calculateSize(quint64 size);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->actionNew_Window->setShortcut(QKeySequence::New);

    ui->SpecialFolderActions->setVisible(false);
    ui->SpecialFolderWarningFrame->setVisible(false);
    ui->UndoPanel->setVisible(false);
    ui->UndoIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(16, 16));
    ui->SpecialFolderWarningIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));

    ui->fileInfoPane->setParent(this);
    ui->fileInfoPane->setGeometry(0, this->height(), this->width(), this->height());

    ui->filesTable->setColumnCount(4);
    ui->filesTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Size" << "Date Modified");
    ui->filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->fileMessage->setVisible(false);

    ui->mainToolBar->addWidget(ui->currentDirBar);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadList()));

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(reloadLeftList()));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(reloadLeftList()));

    connect(ui->OwnerRead, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->OwnerWrite, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->OwnerExecute, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->GroupRead, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->GroupWrite, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->GroupExecute, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->OtherRead, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->OtherWrite, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));
    connect(ui->OtherExecute, SIGNAL(toggled(bool)), this, SLOT(setFilePermissions()));

    currentDir = QDir::home();

    this->reloadList();
    this->reloadLeftList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isDirectory(QString dir) {
    return QDir(dir).exists();
}

bool MainWindow::isTrashFolder() {
    if (currentDir.path() == QDir::homePath() + "/.local/share/Trash/files") {
        return true;
    } else {
        return false;
    }
}

bool MainWindow::navigate(QString path) {
    cancelMount = true;
    ui->fileMessage->setVisible(false);
    ui->FileListFrame->setVisible(true);

    ui->filesTable->clearSelection();
    pathHistory.append(path);
    currentDir = QDir(path);
    watcher->directories().clear();
    watcher->addPath(path);
    reloadList();

    if (!currentDir.exists()) {
        showErrorMessage("This folder doesn't exist.");
        return false;
    } else if (!currentDir.isReadable()) {
        showErrorMessage("Can't read this folder.");
        return false;
    } else {
        return true;
    }
}

void MainWindow::showErrorMessage(QString message) {
    ui->messageIcon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(16, 16));
    ui->message->setText(message);
    ui->messageFrame->setVisible(true);
}

void MainWindow::showInfoMessage(QString message) {
    ui->messageIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(16, 16));
    ui->message->setText(message);
    ui->messageFrame->setVisible(true);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (ui->fileInfoPane->y() == 0) {
        ui->fileInfoPane->setGeometry(0, 0, event->size().width(), event->size().height());
    } else {
        ui->fileInfoPane->setGeometry(0, event->size().height(), event->size().width(), event->size().height());
    }
}

void MainWindow::reloadList() {
    ui->messageFrame->setVisible(false);
    QStorageInfo StorageInfo(currentDir.path());

    if (currentDir.exists()) {
        QFileInfoList info = currentDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | (ui->actionShow_Hidden_Files->isChecked() ? QDir::Hidden : (QDir::Filters) 0), QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);
        ui->filesTable->setRowCount(info.count());

        if (info.count() == 0) {
            showInfoMessage("There's nothing in this folder.");
        } else {
            for (int i = 0; i < info.count(); i++) {
                QTableWidgetItem* filename = new QTableWidgetItem();
                if (info[i].fileName().startsWith(".")) {
                    filename->setText(info[i].fileName());
                } else {
                    filename->setText(info[i].baseName());
                }
                filename->setData(Qt::UserRole, info[i].filePath());
                filename->setData(Qt::UserRole + 1, info[i].fileName());
                if (info[i].isDir()) {
                    if (info[i].path() == QDir::homePath() && info[i].fileName() == "Documents") {
                        filename->setIcon(QIcon::fromTheme("folder-documents"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Downloads") {
                            filename->setIcon(QIcon::fromTheme("folder-downloads"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Music") {
                            filename->setIcon(QIcon::fromTheme("folder-music"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Videos") {
                            filename->setIcon(QIcon::fromTheme("folder-videos"));
                    } else if (info[i].path() == QDir::homePath() && info[i].fileName() == "Pictures") {
                            filename->setIcon(QIcon::fromTheme("folder-pictures"));
                    } else {
                        filename->setIcon(QIcon::fromTheme("folder"));
                    }
                } else {
                    filename->setIcon(QIcon::fromTheme(mimeDatabase.mimeTypeForFile(info[i].filePath()).iconName()));
                }

                QTableWidgetItem* description = new QTableWidgetItem();
                description->setText(mimeDatabase.mimeTypeForFile(info[i].filePath()).comment());

                QTableWidgetItem* size = new QTableWidgetItem();
                if (!isDirectory(info[i].filePath())) {
                    size->setText(calculateSize(info[i].size()));
                }

                QTableWidgetItem* date = new QTableWidgetItem();
                date->setText(info[i].lastModified().toString("dd/MM/yy hh:mm:ss"));

                if (info[i].isExecutable() && info[i].isFile() && StorageInfo.fileSystemType() == "ext4") {
                    QBrush background(QColor::fromRgb(0, 200, 0, 100));
                    filename->setBackground(background);
                    description->setBackground(background);
                    size->setBackground(background);
                    date->setBackground(background);
                } else if (info[i].isHidden()) {
                    QBrush disabledColor = ui->filesTable->palette().brush(QPalette::Disabled, QPalette::Foreground);
                    filename->setForeground(disabledColor);
                    description->setForeground(disabledColor);
                    size->setForeground(disabledColor);
                    date->setForeground(disabledColor);
                }

                ui->filesTable->setItem(i, 0, filename);
                ui->filesTable->setItem(i, 1, description);
                ui->filesTable->setItem(i, 2, size);
                ui->filesTable->setItem(i, 3, date);
            }
        }

        ui->SizeLabel->setText(calculateSize(StorageInfo.bytesAvailable()) + " out of " + calculateSize(StorageInfo.bytesTotal()) + " available.");

        //Hide Special Folder buttons
        ui->EmptyTrashButton->setVisible(false);

        //Determine if we are in a special folder
        if (isTrashFolder()) {
            ui->SpecialFolderActions->setVisible(true);
            ui->SpecialFolderName->setText("Trash");
            ui->EmptyTrashButton->setVisible(true);
            ui->SpecialFolderInfo->setVisible(false);
            ui->SpecialFolderWarningFrame->setVisible(false);
        } else if (StorageInfo.rootPath() == currentDir.path()) {
            //This folder is a mountpoint. Show drive info.
            ui->SpecialFolderActions->setVisible(true);


            QStringList info;
            info.append("Size: " + calculateSize(StorageInfo.bytesTotal()));
            info.append("Remaining Space: " + calculateSize(StorageInfo.bytesAvailable()));

            if (StorageInfo.name() == "") {
                if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios")) {
                    QProcess iosName;
                    iosName.start("idevice_id " + currentDir.path().replace("/", "").right(40));
                    iosName.waitForFinished();

                    QString name(iosName.readAll());
                    name = name.trimmed();

                    if (name == "") {
                        ui->SpecialFolderName->setText("iOS Device");
                    } else {
                        ui->SpecialFolderName->setText(name);
                    }
                } else if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp")) {
                    ui->SpecialFolderName->setText("MTP Device");
                }
            } else {
                ui->SpecialFolderName->setText(StorageInfo.name());
                info.append("Device: " + StorageInfo.device());
            }
            ui->SpecialFolderInfo->setText(info.join("\n"));

            if ((qreal) StorageInfo.bytesAvailable() / (qreal) StorageInfo.bytesTotal() < 0.05) {
                ui->SpecialFolderWarningLabel->setText("This drive is running low on space. Delete or move some files.");
                ui->SpecialFolderWarningFrame->setVisible(true);
            } else {
                ui->SpecialFolderWarningFrame->setVisible(false);
            }

            ui->SpecialFolderInfo->setVisible(true);
        } else {
            ui->SpecialFolderActions->setVisible(false);
        }

        if (StorageInfo.rootPath() == currentDir.path() && !currentDir.isRoot()) {
            ui->actionUnmount->setVisible(true);
        } else {
            ui->actionUnmount->setVisible(false);
        }

        /*if (StorageInfo.isRoot()) {
            ui->currentDirBar->setText("Root" + currentDir.path().replace("/", " > "));
        } else {
            ui->currentDirBar->setText(StorageInfo.displayName() + currentDir.path().remove(StorageInfo.rootPath()).replace("/", " > "));
        }*/

        if (isTrashFolder()) {
            //This is the trash folder.
            ui->currentDirBar->setText("Trash");
            this->setWindowFilePath(currentDir.path());
            this->setWindowTitle("Trash - theFile");
        } else if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios")) {
            ui->currentDirBar->setText("iOS Device " + currentDir.path().remove(QDir::homePath() + "/.thefile/ios"));
            this->setWindowFilePath(currentDir.path());
            this->setWindowTitle("iOS Device " + currentDir.path().remove(QDir::homePath() + "/.thefile/ios") + " - theFile");
        } else if (currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp")) {
            ui->currentDirBar->setText("MTP Device " + currentDir.path().remove(QDir::homePath() + "/.thefile/mtp"));
            this->setWindowFilePath(currentDir.path());
            this->setWindowTitle("iOS Device " + currentDir.path().remove(QDir::homePath() + "/.thefile/mtp") + " - theFile");

        } else {
            ui->currentDirBar->setText(currentDir.path());
            this->setWindowFilePath(currentDir.path());
            this->setWindowTitle(currentDir.path() + " - theFile");
        }
    } else {
        ui->filesTable->setRowCount(0);
    }

    ui->UndoPanel->setVisible(false);
}

void MainWindow::reloadLeftList() {
    ui->placesTable->clear();

    //Add Common Folders
    {
        QListWidgetItem* homeDir = new QListWidgetItem();
        homeDir->setText("Home");
        homeDir->setIcon(QIcon::fromTheme("user-home"));
        homeDir->setData(Qt::UserRole, "dir");
        homeDir->setData(Qt::UserRole + 1, QDir::homePath());
        ui->placesTable->addItem(homeDir);

        QListWidgetItem* rootDir = new QListWidgetItem();
        rootDir->setText("Root");
        rootDir->setIcon(QIcon::fromTheme("computer"));
        rootDir->setData(Qt::UserRole, "dir");
        rootDir->setData(Qt::UserRole + 1, QDir::rootPath());
        ui->placesTable->addItem(rootDir);

        QListWidgetItem* trashDir = new QListWidgetItem();
        trashDir->setText("Trash");
        trashDir->setIcon(QIcon::fromTheme("user-trash"));
        trashDir->setData(Qt::UserRole, "dir");
        trashDir->setData(Qt::UserRole + 1, QDir::homePath() + "/.local/share/Trash/files");
        ui->placesTable->addItem(trashDir);
    }

    //Add User Places
    int placesCount = settings.beginReadArray("fav");
    if (placesCount > 0) {
        QListWidgetItem* sep = new QListWidgetItem();
        sep->setSizeHint(QSize(50, 1));
        sep->setFlags(Qt::NoItemFlags);
        ui->placesTable->addItem(sep);

        QFrame *sepLine = new QFrame();
        sepLine->setFrameShape(QFrame::HLine);
        ui->placesTable->setItemWidget(sep, sepLine);

        for (int i = 0; i < placesCount; i++) {
            settings.setArrayIndex(i);
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(settings.value("name").toString());
            item->setIcon(QIcon::fromTheme("folder"));
            item->setData(Qt::UserRole, "dir");
            item->setData(Qt::UserRole + 1, settings.value("path"));
            ui->placesTable->addItem(item);
        }
    }
    settings.endArray();

    //Load Drives
    MountPoints.clear();

    QDBusInterface udisksBlocks("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices", "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus());
    QDBusReply<QString> reply = udisksBlocks.call("Introspect");
    if (reply.isValid()) {
        QListWidgetItem* sep = new QListWidgetItem();
        sep->setSizeHint(QSize(50, 1));
        sep->setFlags(Qt::NoItemFlags);
        ui->placesTable->addItem(sep);

        QFrame *sepLine = new QFrame();
        sepLine->setFrameShape(QFrame::HLine);
        ui->placesTable->setItemWidget(sep, sepLine);

        QXmlStreamReader xmlReader(reply.value());
        //QStringList drives;
        while (!xmlReader.atEnd()) {
            xmlReader.readNext();
            if (xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name().toString() == "node") {
                QString name = xmlReader.attributes().value("name").toString();
                if (name != "") {
                    QDBusInterface udisksBlock("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + name, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
                    if (!udisksBlock.property("hintIgnore").toBool() && udisksBlock.property("IdUsage").toString() == "filesystem") {
                        QString text = udisksBlock.property("IdLabel").toString();
                        if (text == "") {
                            text = calculateSize(udisksBlock.property("Size").toLongLong()) + " Drive";
                        }

                        QIcon icon;
                        QDBusInterface udisksDrive("org.freedesktop.UDisks2", udisksBlock.property("Drive").value<QDBusObjectPath>().path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
                        if (udisksDrive.property("Optical").toBool()) {
                            icon = QIcon::fromTheme("drive-optical");
                        } else if (udisksDrive.property("Removable").toBool()) {
                            icon = QIcon::fromTheme("drive-removable-media");
                        } else {
                            icon = QIcon::fromTheme("drive-harddisk");
                        }

                        QListWidgetItem* item = new QListWidgetItem();

                        QDBusInterface udisksFileSystem("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + name, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                        QDBusMessage MountPoints = udisksFileSystem.call("Get", "org.freedesktop.UDisks2.Filesystem", "MountPoints");
                        QDBusArgument args = MountPoints.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
                        args.beginArray();

                        if (args.atEnd()) {
                            QPainter p;
                            QPixmap temp = icon.pixmap(16,16);
                            p.begin(&temp);
                            p.drawPixmap(8,8,8,8,QIcon::fromTheme("emblem-unmounted").pixmap(8,8));
                            p.end();
                            icon = QIcon(temp);
                        } else {
                            QPainter p;
                            QPixmap temp = icon.pixmap(16,16);
                            p.begin(&temp);
                            p.drawPixmap(8,8,8,8,QIcon::fromTheme("emblem-mounted").pixmap(8,8));
                            p.end();
                            icon = QIcon(temp);

                            while (!args.atEnd()) {
                                QByteArray data;
                                args >> data;
                                this->MountPoints.insert(data, name);
                            }
                        }

                        args.endArray();

                        item->setText(text);
                        item->setData(Qt::UserRole, "filesystem");
                        item->setData(Qt::UserRole + 1, name);
                        item->setIcon(icon);

                        ui->placesTable->addItem(item);
                    }
                }
            }
        }
    }

    //Prepare devices seperator
    QListWidgetItem* sepDev = new QListWidgetItem();
    sepDev->setSizeHint(QSize(50, 1));
    sepDev->setFlags(Qt::NoItemFlags);
    ui->placesTable->addItem(sepDev);

    QFrame *sepLineDev = new QFrame();
    sepLineDev->setFrameShape(QFrame::HLine);
    bool hasItem = false;
    ui->placesTable->setItemWidget(sepDev, sepLineDev);

    //Load external devices
    if (QFile("/usr/bin/jmtpfs").exists()) {
        //Detect MTP Devices
        QProcess mtpDetection;
        mtpDetection.start("jmtpfs -l");
        mtpDetection.waitForStarted();

        while (mtpDetection.state() == QProcess::Running) {
            QApplication::processEvents();
        }

        QString output = mtpDetection.readAll();
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
                    ui->placesTable->addItem(item);
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

        QProcess iosDev;
        iosDev.start("idevice_id -l");
        iosDev.waitForStarted();

        while (iosDev.state() == QProcess::Running) {
            QApplication::processEvents();
        }

        QString output(iosDev.readAll());
        for (QString line : output.split("\n")) {
            if (line != "") {
                if (!line.startsWith("ERROR:")) {
                    QListWidgetItem* item = new QListWidgetItem();
                    QProcess iosName;
                    iosName.start("idevice_id " + line);
                    iosName.waitForFinished();

                    QString name(iosName.readAll());
                    name = name.trimmed();

                    if (name == "") {
                        item->setText("iOS Device");
                    } else {
                        item->setText(name + " (iOS)");
                    }

                    item->setIcon(QIcon::fromTheme("smartphone"));
                    item->setData(Qt::UserRole, "ios");
                    item->setData(Qt::UserRole + 1, line);
                    ui->placesTable->addItem(item);
                    hasItem = true;
                }
            }
        }
    }

    //If there are no devices, delete the seperator
    if (!hasItem) {
        sepLineDev->deleteLater();
        delete sepDev;
    }
}

void MainWindow::on_currentDirBar_returnPressed()
{
    navigate(ui->currentDirBar->text());
}

void MainWindow::on_actionGo_Up_triggered()
{
    if (isTrashFolder()) {
        currentDir = QDir::home();
    } else {
        currentDir.cdUp();
    }
    reloadList();
}

void MainWindow::on_filesTable_cellActivated(int row, int column)
{

}

void MainWindow::on_filesTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)

    if (isTrashFolder()) { //File is in trash. Don't allow opening.
        QMessageBox::information(this, "File in trash", "This file/folder is in the trash. Restore the file to open it.", QMessageBox::Ok, QMessageBox::Ok);
    } else {
        QString filename = ui->filesTable->item(row, 0)->data(Qt::UserRole).toString();
        if (isDirectory(filename)) {
            navigate(filename);
        } else if (QFileInfo(filename).isExecutable()) {
            QProcess::startDetached(filename);
        } else {
            QProcess::startDetached("xdg-open", QStringList() << filename);
        }
    }
}


void MainWindow::on_placesTable_itemActivated(QListWidgetItem *item)
{
    cancelMount = true;
    ui->fileMessage->setVisible(false);
    ui->FileListFrame->setVisible(true);

    if (item->data(Qt::UserRole).toString() == "filesystem") { //Filesystem Deviec
        QDBusInterface udisksFileSystemProperties("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + item->data(Qt::UserRole + 1).toString(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusMessage MountPoints = udisksFileSystemProperties.call("Get", "org.freedesktop.UDisks2.Filesystem", "MountPoints");
        QDBusArgument args = MountPoints.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
        args.beginArray();

        if (args.atEnd()) {
            QDBusInterface udisksFileSystem("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + item->data(Qt::UserRole + 1).toString(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
            QDBusReply<QString> reply = udisksFileSystem.call("Mount", QVariantMap());
            navigate(reply);
        } else {
            QByteArray data;
            args >> data;
            navigate(QString(data));
        }

        args.endArray();
    } else if (item->data(Qt::UserRole).toString() == "dir") { //Docked Item
        navigate(item->data(Qt::UserRole + 1).toString());
    } else if (item->data(Qt::UserRole).toString() == "mtp") { //MTP Device
        qDebug() << "Mounting MTP device " + item->data(Qt::UserRole + 1).toString() + ", " + item->data(Qt::UserRole + 2).toString();

        QString mtpDirName = "mtp" + item->data(Qt::UserRole + 1).toString() + "," + item->data(Qt::UserRole + 2).toString();
        QDir::home().mkdir(".thefile");
        QDir(QDir::homePath() + "/.thefile").mkdir(mtpDirName);
        QProcess mountProcess;

        bool mounted = false;
        for (QString file : QDir(QDir::homePath() + "/.thefile/" + mtpDirName).entryList()) {
            if (file != "." && file != "..") {
                mounted = true;
            }
        }
        if (!mounted) {
            mountProcess.start("jmtpfs " + QDir::homePath() + "/.thefile/" + mtpDirName + " -device=" + item->data(Qt::UserRole + 1).toString() + "," + item->data(Qt::UserRole + 2).toString());
            mountProcess.waitForStarted();

            while (mountProcess.state() == QProcess::Running) {
                QApplication::processEvents();
            }
        }

        navigate(QDir::homePath() + "/.thefile/" + mtpDirName);
    } else if (item->data(Qt::UserRole).toString() == "ios") { //iOS Device
        QString id = item->data(Qt::UserRole + 1).toString();
        qDebug() << "Mounting iOS Device " + id;

        bool devicePairSuccess = false;
        cancelMount = false;
        do {
            QProcess pairProcess;
            pairProcess.start("idevicepair -u " + id + " validate");
            pairProcess.waitForFinished();
            if (pairProcess.readAll().startsWith("ERROR")) {
                pairProcess.start("idevicepair -u " + id + " pair");
                pairProcess.waitForStarted();

                while (pairProcess.state() == QProcess::Running) {
                    QApplication::processEvents();
                }

                QString pairOutput(pairProcess.readAll());
                if (pairOutput.startsWith("ERROR:")) {
                    if (pairOutput.contains("Please enter the passcode")) { // Ask user to unlock device
                        ui->fileMessageTitle->setText("Unlock Device");
                        ui->fileMessageText->setText("You'll need to unlock your iOS Device before theFile can access it. Enter the passcode on your device.");
                    } else if (pairOutput.contains("Please accept the trust dialog")) { //Ask user to trust PC
                        ui->fileMessageTitle->setText("Trust this PC");
                        ui->fileMessageText->setText("You'll need to trust this PC on your device before theFile can access it. Accept the trust dialog "
                                                     "on your device, and wait for theFile to detect that the device trusts this PC.");
                    } else if (pairOutput.contains("user denied the trust dialog")) { //User did not trust PC
                        ui->fileMessageTitle->setText("Device Not Trusted");
                        ui->fileMessageText->setText("The iOS Device does not trust this PC. Click \"Cancel,\" unplug the device and plug it back in.");
                    } else { //Generic Error
                        ui->fileMessageTitle->setText("iOS Error");
                        ui->fileMessageText->setText("An error occurred trying to pair with the device:\n\n" + pairOutput);
                    }
                    ui->fileMessage->setVisible(true);
                    ui->FileListFrame->setVisible(false);
                } else {
                    devicePairSuccess = true;
                }
            } else {
                devicePairSuccess = true;
            }
        } while (!devicePairSuccess && !cancelMount);

        ui->fileMessage->setVisible(false);
        ui->FileListFrame->setVisible(true);

        if (cancelMount) {
            cancelMount = false;
            return;
        }

        QString iosDirName = "ios" + id;
        QDir::home().mkdir(".thefile");
        QDir(QDir::homePath() + "/.thefile").mkdir(iosDirName);

        QProcess mountProcess;

        bool mounted = false;
        for (QString file : QDir(QDir::homePath() + "/.thefile/" + iosDirName).entryList()) {
            if (file != "." && file != "..") {
                mounted = true;
            }
        }
        if (!mounted) {
            mountProcess.start("ifuse -o ro " + QDir::homePath() + "/.thefile/" + iosDirName + " -u " + id);
            mountProcess.waitForStarted();

            while (mountProcess.state() == QProcess::Running) {
                QApplication::processEvents();
            }
        }

        navigate(QDir::homePath() + "/.thefile/" + iosDirName);
    }
}

void MainWindow::on_actionNew_Window_triggered()
{
    (new MainWindow())->show();
}

void MainWindow::on_actionBookmark_triggered()
{
    QString name = QInputDialog::getText(this, "Name", "Enter a name for this place:", QLineEdit::Normal, currentDir.dirName());
    if (name != "") {
        int placesCount = settings.beginReadArray("fav");
        QStringList names, paths;
        for (int i = 0; i < placesCount; i++) {
            settings.setArrayIndex(i);
            names.append(settings.value("name").toString());
            paths.append(settings.value("path").toString());
        }
        settings.endArray();

        settings.beginWriteArray("fav");
        for (int i = 0; i < placesCount; i++) {
            settings.setArrayIndex(i);
            settings.setValue("name", names[i]);
            settings.setValue("path", paths[i]);
        }
        settings.setArrayIndex(placesCount);
        settings.setValue("name", name);
        settings.setValue("path", currentDir.path());
        settings.endArray();

        reloadLeftList();
    }
}

void MainWindow::on_actionUnmount_triggered()
{
    QMenu* menu = new QMenu();

    QString device = MountPoints.value(currentDir.path());
    QDBusInterface udisksBlock("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + device, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    QDBusInterface udisksDrive("org.freedesktop.UDisks2", udisksBlock.property("Drive").value<QDBusObjectPath>().path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

    bool canPowerOff = udisksDrive.property("CanPowerOff").toBool();
    if (canPowerOff) {
        menu->addAction(QIcon::fromTheme("removable-media"), "Prepare for Removal", [=, &udisksBlock, &udisksDrive]() {
            QDBusInterface udisksFileSystem("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + device, "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
            udisksFileSystem.call("Unmount", QVariantMap());
            udisksDrive.call("PowerOff", QVariantMap());

            /*if (QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + device, "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus()).isValid()) {
                QMessageBox::information(this, "Remove", "Couldn't power off the drive.");
            } else {
                QMessageBox::information(this, "Remove", "It is now safe to remove the drive.");
            }*/
            navigate(QDir::homePath());
        });
    }

    menu->addAction(QIcon::fromTheme("media-eject"), "Unmount", [=]() {
        if (device == "") {
            if ((currentDir.path().startsWith(QDir::homePath() + "/.thefile/mtp") &&
                        !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/mtp").length()).contains("/")) ||
                        (currentDir.path().startsWith(QDir::homePath() + "/.thefile/ios") &&
                        !currentDir.path().remove(0, QString(QDir::homePath() + "/.thefile/ios").length()).contains("/"))) {
                    QProcess::execute("fusermount -u " + currentDir.path());
                    currentDir.removeRecursively();
                    navigate(QDir::homePath());
            } else {
                showErrorMessage("Can't unmount this drive.");
            }
        } else {
            QDBusInterface udisksFileSystem("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + device, "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
            udisksFileSystem.call("Unmount", QVariantMap());
            navigate(QDir::homePath());
        }
    });

    menu->exec(QCursor::pos());
}

void MainWindow::on_actionNew_Folder_triggered()
{
    ui->filesTable->setRowCount(ui->filesTable->rowCount() + 1);
    QTableWidgetItem *newItem = new QTableWidgetItem();
    newItem->setText("New Folder");
    newItem->setIcon(QIcon::fromTheme("folder"));
    ui->filesTable->setItem(ui->filesTable->rowCount() - 1, 0, newItem);
    renamingFile = "mkdir";
    ui->filesTable->editItem(newItem);
    ui->filesTable->scrollToItem(newItem);
}

void MainWindow::on_filesTable_itemChanged(QTableWidgetItem *item)
{
    if (renamingFile != "") {
        if (QFile(currentDir.path() + "/" + item->text()).exists() || QDir(currentDir.path() + "/" + item->text()).exists()) {
            showInfoMessage("There's already a file/folder named " + item->text() + " in this folder. Do something with that first!");
            item->setText(renamingFile);
            return;
        }

        if (renamingFile == "mkdir") {
            currentDir.mkdir(item->text());
        } else {
            QFile(currentDir.path() + "/" + renamingFile).rename(currentDir.path() + "/" + item->text());
        }
        renamingFile = "";
        reloadList();
    }
}

void MainWindow::on_filesTable_customContextMenuRequested(const QPoint &pos)
{
    //Prepare a context menu
    QMenu *cx = new QMenu(this);

    //Determine if this is the trash folder
    if (isTrashFolder()) {
        if (ui->filesTable->selectedItems().count() > 0) {
            cx->addSection("For all selected items");
            cx->addAction(ui->actionRestore_from_Trash);
            cx->addAction(ui->actionDelete_Permanantly);
        }
    } else {
        if (ui->filesTable->selectedItems().count() / 4 == 1) {
            cx->addSection("For \"" + ui->filesTable->selectedItems().at(0)->text() + "\"");
            cx->addAction(ui->actionCopy);
            cx->addAction(ui->actionCut);
            cx->addSeparator();
            cx->addAction(ui->actionRename);
            cx->addAction(ui->actionMove_To_Trash);
            cx->addAction(ui->actionDelete_Permanantly);
            cx->addAction(ui->actionMore_Info);
        } else if (ui->filesTable->selectedItems().count() / 4 > 1) {
            cx->addSection("For all selected items");
            cx->addAction(ui->actionCopy);
            cx->addAction(ui->actionCut);
            cx->addSeparator();
            cx->addAction(ui->actionMove_To_Trash);
            cx->addAction(ui->actionDelete_Permanantly);
            cx->addAction(ui->actionMore_Info);
        }

        cx->addSection("For folder \"" + currentDir.dirName() + "\"");
        cx->addAction(ui->actionNew_Folder);
    }

    cx->exec(ui->filesTable->mapToGlobal(pos));
}

void MainWindow::on_actionRename_triggered()
{
    ui->filesTable->editItem(ui->filesTable->selectedItems().at(0));
    renamingFile = ui->filesTable->selectedItems().at(0)->data(Qt::UserRole + 1).toString();
}

void MainWindow::on_filesTable_itemSelectionChanged()
{
    if (ui->filesTable->selectedItems().count() == 0) {
        ui->actionRename->setEnabled(false);
        ui->actionMove_To_Trash->setEnabled(false);
    } else {
        ui->actionRename->setEnabled(true);
        ui->actionMove_To_Trash->setEnabled(true);
    }

    if (isTrashFolder()) {
        ui->actionMove_To_Trash->setVisible(false);
    } else {
        ui->actionMove_To_Trash->setVisible(true);
    }
}

void MainWindow::on_actionShow_Hidden_Files_triggered(bool checked)
{
    reloadList();
}

void MainWindow::on_actionMove_To_Trash_triggered()
{
    FilesToUndo.clear();

    QDir trash = QDir::home();
    trash.cd(".local/share/");
    trash.mkdir("Trash");
    trash.cd("Trash");
    trash.mkdir("files");
    trash.mkdir("info");

    QFile directorySizes(trash.path() + "/directorysizes");
    directorySizes.open(QFile::Append);

    for (QTableWidgetItem *item : ui->filesTable->selectedItems()) {
        if (item->column() == 0) {
            QString fileLocation = item->data(Qt::UserRole).toString();
            if (QFile(fileLocation).exists()) {
                QFile(fileLocation).rename(trash.path() + "/files/" + item->data(Qt::UserRole + 1).toString());
            } else {
                currentDir.rename(item->data(Qt::UserRole + 1).toString(), trash.path() + "/files/ " + item->data(Qt::UserRole + 1).toString());
            }

            QFile trashinfo(trash.path() + "/info/" + item->data(Qt::UserRole + 1).toString() + ".trashinfo");
            trashinfo.open(QFile::WriteOnly);
            trashinfo.write(QString("[Trash Info]\n").toUtf8());
            trashinfo.write(QString("Path=" + fileLocation + "\n").toUtf8());
            trashinfo.write(QString("DeletionDate=" + QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss") + "\n").toUtf8());
            trashinfo.close();

            FilesToUndo.append(item->text());
        }
    }

    reloadList();

    ui->UndoMessage->setText("It's gone.");
    ui->UndoPanel->setVisible(true);

    ui->filesTable->clearSelection();
}

void MainWindow::on_UndoButton_clicked()
{
    restoreFilesFromTrash(FilesToUndo);
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

    reloadList();
}
void MainWindow::on_actionRestore_from_Trash_triggered()
{
    QStringList filesToRestore;
    for (QTableWidgetItem *item : ui->filesTable->selectedItems()) {
        if (item->column() == 0) {
            filesToRestore.append(item->data(Qt::UserRole + 1).toString());
        }
    }

    restoreFilesFromTrash(filesToRestore);
}

void MainWindow::on_actionDelete_Permanantly_triggered()
{
    if (QMessageBox::question(this, "Delete File", "You're about to permanantly delete files. These files can't be recovered.", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        for (QTableWidgetItem *item : ui->filesTable->selectedItems()) {
            if (item->column() == 0) {
                if (!QFile(item->data(Qt::UserRole).toString()).remove()) {
                    QDir dirToRemove(item->data(Qt::UserRole).toString());
                    dirToRemove.removeRecursively();
                }

                if (isTrashFolder()) { //This file is in trash, also remove metadata
                    QDir trash = QDir::home();
                    trash.cd(".local/share/Trash");
                    QFile info(trash.path() + "/info/" + item->data(Qt::UserRole + 1).toString() + ".trashinfo");
                    info.remove();
                }
            }
        }
    }

    reloadList();
}

void MainWindow::on_EmptyTrashButton_clicked()
{
    ui->filesTable->selectAll();
    ui->actionDelete_Permanantly->trigger();
}

void MainWindow::on_actionMore_Info_triggered()
{
    if (ui->filesTable->selectedItems().count() / 4 == 1) {
        ui->fileInfoLabel->setText(ui->filesTable->selectedItems().first()->data(Qt::UserRole + 1).toString());

        QFileInfo fileInfo(ui->filesTable->selectedItems().first()->data(Qt::UserRole).toString());
        QStringList info;
        info.append("Location: " + fileInfo.path());

        if (fileInfo.isDir()) {
            //TODO: Calculate size
        } else {
            info.append("Size: " + calculateSize(fileInfo.size()) + " (" + QString::number(fileInfo.size()) + " bytes)");
        }

        info.append("Date Created: " + fileInfo.created().toString("dddd, dd MMMM yyyy, hh:mm:ss"));
        info.append("Date Modified: " + fileInfo.lastModified().toString("dddd, dd MMMM yyyy, hh:mm:ss"));
        info.append("Date Accessed: " + fileInfo.lastRead().toString("dddd, dd MMMM yyyy, hh:mm:ss"));


        info.append("");
        info.append("Owner: " + fileInfo.owner() + " (" + QString::number(fileInfo.ownerId()) + ")");
        info.append("Group: " + fileInfo.group() + " (" + QString::number(fileInfo.groupId()) + ")");
        ui->fileInfoText->setText(info.join("\n"));

        ui->OwnerRead->setChecked(fileInfo.permission(QFile::ReadOwner));
        ui->OwnerWrite->setChecked(fileInfo.permission(QFile::WriteOwner));
        ui->OwnerExecute->setChecked(fileInfo.permission(QFile::ExeOwner));
        ui->GroupRead->setChecked(fileInfo.permission(QFile::ReadGroup));
        ui->GroupWrite->setChecked(fileInfo.permission(QFile::WriteGroup));
        ui->GroupExecute->setChecked(fileInfo.permission(QFile::ExeGroup));
        ui->OtherRead->setChecked(fileInfo.permission(QFile::ReadOther));
        ui->OtherWrite->setChecked(fileInfo.permission(QFile::WriteOther));
        ui->OtherExecute->setChecked(fileInfo.permission(QFile::ExeOther));
    } else {
        ui->fileInfoLabel->setText("Info");

        QStringList info;
        info.append("Location: " + currentDir.path());
        info.append("Number of Files: " + QString::number(ui->filesTable->selectedItems().count() / 4));
        ui->fileInfoText->setText(info.join("\n"));
    }

    QPropertyAnimation* anim = new QPropertyAnimation(ui->fileInfoPane, "geometry");
    anim->setStartValue(ui->fileInfoPane->geometry());
    anim->setEndValue(QRect(0, 0, this->size().width(), this->size().height()));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::on_fileInfoDone_clicked()
{
    reloadList();

    QPropertyAnimation* anim = new QPropertyAnimation(ui->fileInfoPane, "geometry");
    anim->setStartValue(ui->fileInfoPane->geometry());
    anim->setEndValue(QRect(0, this->height(), this->size().width(), this->size().height()));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::setFilePermissions() {
    QFile::Permissions setPermissions;

    if (ui->OwnerRead->isChecked()) setPermissions |= QFile::ReadOwner;
    if (ui->OwnerWrite->isChecked()) setPermissions |= QFile::WriteOwner;
    if (ui->OwnerExecute->isChecked()) setPermissions |= QFile::ExeOwner;
    if (ui->GroupRead->isChecked()) setPermissions |= QFile::ReadGroup;
    if (ui->GroupWrite->isChecked()) setPermissions |= QFile::WriteGroup;
    if (ui->GroupExecute->isChecked()) setPermissions |= QFile::ExeGroup;
    if (ui->OtherRead->isChecked()) setPermissions |= QFile::ReadOther;
    if (ui->OtherWrite->isChecked()) setPermissions |= QFile::WriteOther;
    if (ui->OtherExecute->isChecked()) setPermissions |= QFile::ExeOther;

    for (QTableWidgetItem* item : ui->filesTable->selectedItems()) {
        if (item->column() == 0) {
            QFile file(item->data(Qt::UserRole).toString());
            file.setPermissions(setPermissions);
        }
    }
}

void MainWindow::on_actionCopy_triggered()
{
    QMimeData* mimeData = new QMimeData();
    QByteArray files;
    files.append("copy");

    for (QTableWidgetItem* item : ui->filesTable->selectedItems()) {
        if (item->column() == 0) {
            files.append("\n" + QUrl::fromLocalFile(item->data(Qt::UserRole).toString()).toEncoded());
        }
    }
    files.append("\0");

    mimeData->setData("x-special/gnome-copied-files", files);
    QApplication::clipboard()->setMimeData(mimeData);
}

void MainWindow::on_actionCut_triggered()
{
    QMimeData* mimeData = new QMimeData();
    QByteArray files;
    files.append("cut");

    for (QTableWidgetItem* item : ui->filesTable->selectedItems()) {
        if (item->column() == 0) {
            files.append("\n" + QUrl::fromLocalFile(item->data(Qt::UserRole).toString()).toEncoded());
        }
    }
    files.append("\0");

    mimeData->setData("x-special/gnome-copied-files", files);
    QApplication::clipboard()->setMimeData(mimeData);
}

void MainWindow::on_actionPaste_triggered()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    qDebug() << mimeData->formats();
    qDebug() << mimeData->data("x-special/gnome-copied-files");

    QStringList files = QString(mimeData->data("x-special/gnome-copied-files")).split("\n");
    if (files.first() == "copy") {
        files.removeFirst();
        new CopyJob(files, currentDir.path());
    }
}

void MainWindow::on_fileMessageCancel_clicked()
{
    cancelMount = true;
    ui->fileMessage->setVisible(false);
    ui->FileListFrame->setVisible(true);
}
