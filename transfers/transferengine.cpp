#include "transferengine.h"
#include "transferpane.h"

#include <QApplication>

extern QString calculateSize(quint64 size);

TransferObject::TransferObject(QStringList source, QString destination, TransferType type, QObject* parent) : QObject(parent)
{
    QStringList savedSource;
    for (QString src : source) {
        if (src != "") {
            QUrl url(src);
            if (url.isLocalFile()) {
                savedSource.append(url.toLocalFile());
            } else {
                savedSource.append(src);
            }
        }
    }
    this->source = savedSource;

    QUrl destUrl(destination);
    if (destUrl.isLocalFile()) {
        this->dest = destUrl.toLocalFile();
    } else {
        this->dest = destination;
    }
    this->type = type;

    qRegisterMetaType<FileConflictList>();
}

QStringList TransferObject::sources() {
    return source;
}

QString TransferObject::destination() {
    return dest;
}

TransferObject::TransferType TransferObject::transferType() {
    return type;
}

TransferEngine::TransferEngine(QObject* parent) : QObject(parent)
{
    dialog = new TransferDialog();
}

void TransferEngine::addTransfer(TransferObject *transfer) {
    if (!runningTransfers.contains(transfer)) {
        TransferPane* pane = new TransferPane(transfer);

        QStringList sources = transfer->sources();
        QString dest = transfer->destination();

        //Ensure sources and destination are ok
        for (QString source : sources) {
            QFileInfo info(source);
            if (!info.exists()) {
                emit transfer->failed();
                return;
            }

            if (!info.isReadable()) {
                emit transfer->failed();
                return;
            }

            if (info.isDir() && !info.isExecutable()) {
                emit transfer->failed();
                return;
            }
        }

        QFileInfo destInfo(dest);
        if (!destInfo.isDir()) {
            emit transfer->failed();
            return;
        }

        dialog->addTransferPane(pane);
        pane->resize();

        switch (transfer->transferType()) {
            case TransferObject::Copy: {
                TransferCopy* copy = new TransferCopy(sources, dest, pane);
                copy->start();
                break;
            }
            case TransferObject::Move: {
                TransferMove* move = new TransferMove(sources, dest, pane);
                move->start();
                break;
            }
        }
    }
}

TransferDialog::TransferDialog(QWidget *parent) : QDialog(parent)
{
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout(layout);

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    layout->addWidget(scrollArea);

    QWidget* scrollWidget = new QWidget();
    scrollArea->setWidget(scrollWidget);

    transfersLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    scrollWidget->setLayout(transfersLayout);

    resizeHeight();

    this->setWindowTitle("File Transfers");
}

void TransferDialog::addTransferPane(TransferPane *pane) {
    transferPanes.append(pane);
    transfersLayout->addWidget(pane);
    connect(pane, SIGNAL(heightChanged()), this, SLOT(resizeHeight()));

    if (!this->isVisible()) {
        this->show();
    }
    resizeHeight();
}

void TransferDialog::resizeHeight() {
    int totalHeight = 18;
    bool needExtraHeight = false;
    for (TransferPane* pane : transferPanes) {
        totalHeight += pane->height() + 6;
        if (pane->needExtraHeight()) needExtraHeight = true;
    }

    if (needExtraHeight) {
        if (totalHeight > 700) totalHeight = 700;
    } else {
        if (totalHeight > 300) totalHeight = 300;
    }

    //this->setFixedHeight(totalHeight);
    scrollArea->setFixedHeight(totalHeight);
}

TransferThread::TransferThread(QStringList source, QString destination, TransferPane *pane, QObject *parent) : QThread(parent)
{
    this->source = source;
    this->destination = destination;
    this->pane = pane;

    connect(this, SIGNAL(setActionLabelText(QString)), pane, SLOT(setActionLabelText(QString)));
    connect(this, SIGNAL(resolveConflicts(FileConflictList,bool)), pane, SLOT(resolveConflicts(FileConflictList,bool)));
    connect(this, SIGNAL(progress(qulonglong,qulonglong)), pane, SLOT(progress(qulonglong,qulonglong)));
}

void TransferThread::run() {
    //Count files
    emit setActionLabelText("Counting files...");

    for (QString file : source) {
        //Check file conflicts
        QFileInfo info(file);
        this->fileConflicts.append(checkConflicts(file, destination + "/" + info.fileName(), this->bytes));
    }

    if (fileConflicts.count() > 0) {
        QEventLoop* loop = new QEventLoop();
        connect(pane, &TransferPane::conflictsResolved, [=](FileConflictList conflicts) {
            fileConflicts = conflicts;
            loop->exit();
        });
        connect(pane, &TransferPane::cancelTransfer, [=] {
            loop->exit(1);
        });
        emit resolveConflicts(fileConflicts, hasNonSimpleConflicts);
        if (loop->exec() == 1) {
            loop->deleteLater();
            emit done();
            pane->hide();
            return; //Finish the transfer
        }
        loop->deleteLater();

        //Add all the bytes of the files to overwrite/rename
        for (QSharedPointer<FileConflict> c : fileConflicts) {
            if (c->resolution != FileConflict::Skip && c->nature == FileConflict::Conflict) {
                this->bytes += c->bytes;
            }
        }
    }

    //TODO: Sanity checks (permissions, disk space, etc.)

    //Actually start the operation
    bool hasUnresolvedConflicts = false;
    this->doWork(hasUnresolvedConflicts);

    emit done();
    pane->hide();
}

FileConflictList TransferThread::checkConflicts(QString oldFile, QString newFile, qulonglong& bytes) {
    FileConflictList conflicts;
    QFileInfo oldFileInfo(oldFile);
    QFileInfo newFileInfo(newFile);

    if (newFileInfo.exists() && !newFileInfo.isDir() && !newFileInfo.isFile()) {
        //We've got an item we can't handle
        QSharedPointer<FileConflict> conflict(new FileConflict);
        conflict->nature = FileConflict::Warning;
        conflict->explanation = tr("You're trying to transfer an item that is not a directory or a file.");
        conflicts.append(conflict);
        hasNonSimpleConflicts = true;
        return conflicts;
    }

    //Check if the old file is the new file
    if (oldFileInfo.filePath() == newFileInfo.filePath()) {
        //We're trying to replace a file with itself
        QSharedPointer<FileConflict> conflict(new FileConflict);
        conflict->file = oldFileInfo.filePath();
        conflict->conflictingFile = newFileInfo.filePath();
        conflict->nature = FileConflict::Error;
        conflict->explanation = tr("You can't replace %1 with itself").arg(oldFileInfo.fileName());
        conflicts.append(conflict);
        hasNonSimpleConflicts = true;
        return conflicts;
    }

    //Check if the old file is a directory
    if (oldFileInfo.isDir()) {
        if (newFileInfo.exists()) {
            //Ensure the new directory is actually a directory
            if (newFileInfo.isDir()) {
                //Warn that the directory will be written into
                QSharedPointer<FileConflict> conflict(new FileConflict);
                conflict->file = oldFileInfo.filePath();
                conflict->conflictingFile = newFileInfo.filePath();
                conflict->nature = FileConflict::Warning;
                conflict->explanation = tr("By continuing, you'll add items into this existing folder.").arg(oldFileInfo.fileName());
                conflicts.append(conflict);
                hasNonSimpleConflicts = true;

                //Iterate over all the files inside and make sure it doesn't conflict with anything outside
                QDir oldDir(oldFileInfo.filePath());
                QDir newDir(newFileInfo.filePath());
                for (QString file : oldDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
                    conflicts.append(checkConflicts(oldDir.filePath(file), newDir.filePath(file), bytes));
                }
            } else {
                //We're trying to replace an existing directory with a file
                QSharedPointer<FileConflict> conflict(new FileConflict);
                conflict->file = oldFileInfo.filePath();
                conflict->conflictingFile = newFileInfo.filePath();
                conflict->nature = FileConflict::Error;
                conflict->explanation = tr("You can't replace %1 (a directory) with a file.").arg(oldFileInfo.fileName());
                conflicts.append(conflict);
                hasNonSimpleConflicts = true;
                return conflicts;
            }
        }
    } else if (oldFileInfo.isFile()) {
        if (newFileInfo.exists()) {
            if (newFileInfo.isFile()) {
                QSharedPointer<FileConflict> conflict(new FileConflict);
                conflict->file = oldFileInfo.filePath();
                conflict->conflictingFile = newFileInfo.filePath();
                conflict->nature = FileConflict::Conflict;
                conflict->bytes = oldFileInfo.size();
                conflicts.append(conflict);
            } else {
                //We're trying to replace an existing file with a directory
                QSharedPointer<FileConflict> conflict(new FileConflict);
                conflict->file = oldFileInfo.filePath();
                conflict->conflictingFile = newFileInfo.filePath();
                conflict->nature = FileConflict::Error;
                conflict->explanation = tr("You can't replace %1 (a file) with a directory.").arg(oldFileInfo.fileName());
                conflicts.append(conflict);
                hasNonSimpleConflicts = true;
                return conflicts;
            }
        } else {
            bytes += oldFileInfo.size();
        }
    } else {
        //We've got an item we can't handle
        QSharedPointer<FileConflict> conflict(new FileConflict);
        conflict->nature = FileConflict::Warning;
        conflict->explanation = tr("You're trying to transfer an item that is not a directory or a file.");
        conflicts.append(conflict);
        hasNonSimpleConflicts = true;
        return conflicts;
    }
    return conflicts;
}

TransferMove::TransferMove(QStringList source, QString destination, TransferPane *pane, QObject *parent) : TransferThread(source, destination, pane, parent)
{
    this->source = source;
    this->destination = destination;
    this->pane = pane;

    connect(this, SIGNAL(setActionLabelText(QString)), pane, SLOT(setActionLabelText(QString)));
    connect(this, SIGNAL(resolveConflicts(FileConflictList)), pane, SLOT(resolveConflicts(FileConflictList)));
}

void TransferMove::doWork(bool& hadUnresolvedConflicts) {
    emit setActionLabelText("Now moving " + QString::number(numberOfFiles) + " files.");

    //Move each file individually to make sure that file conflicts are handled
    int filesMoved = 0;
    for (QString file : source) {
        //Check file conflicts
        QFileInfo info(file);
        QFile fileObj(file);
        QString newFile = destination + "/" + info.fileName();
        QFileInfo newInfo(newFile);
        if (newInfo.exists()) {
            if (newInfo.isDir()) {
                QDirIterator iterator(file, QDirIterator::Subdirectories);
                while (iterator.hasNext()) {
                    iterator.next();
                    if (iterator.fileName() == "." || iterator.fileName() == "..") {
                        continue;
                    } else {
                        QString newFile = destination + "/" + iterator.filePath().mid(file.lastIndexOf("/") + 1);
                        QFileInfo newInfo(newFile);
                        if (newInfo.exists()) {
                            for (QSharedPointer<FileConflict> conflict : fileConflicts) {
                                if (conflict->file == file) {
                                    if (conflict->resolution == FileConflict::Overwrite) {
                                        QFile(newFile).remove();
                                        fileObj.rename(newFile);
                                        filesMoved++;
                                    }
                                    break;
                                }
                            }
                        } else {
                            fileObj.rename(newFile);
                            filesMoved++;
                        }
                    }
                    numberOfFiles++;

                    emit setActionLabelText(QString::number(numberOfFiles) + " files moved...");
                }
            } else {
                for (QSharedPointer<FileConflict> conflict : fileConflicts) {
                    if (conflict->file == file) {
                        if (conflict->resolution == FileConflict::Overwrite) {
                            QFile(newFile).remove();
                            fileObj.rename(newFile);
                            filesMoved++;
                        }
                        break;
                    }
                }
            }
        } else {
            fileObj.rename(newFile);
            filesMoved++;
        }

        emit setActionLabelText(QString::number(numberOfFiles) + "/" + QString::number(filesMoved) + " files moved...");
    }
}

TransferCopy::TransferCopy(QStringList source, QString destination, TransferPane *pane, QObject *parent) : TransferThread(source, destination, pane, parent)
{

}

void TransferCopy::doWork(bool& hadUnresolvedConflicts) {
    emit setActionLabelText("Now copying " + QString::number(numberOfFiles) + " files.");

    for (QString file : source) {
        //Copy each item recursively
        QFileInfo info(file);
        copyFile(file, destination + "/" + info.fileName(), hadUnresolvedConflicts, this->bytesMoved);
    }

    //We're done!
    return;
}

void TransferCopy::copyFile(QString file, QString destination, bool& hadUnresolvedConflicts, qulonglong& bytesMoved) {
    QFileInfo srcInfo(file);
    QFileInfo dstInfo(destination);

    //TODO: check file conflicts and make sure there are none


    //Check if the old file is a directory
    if (srcInfo.isDir()) {
        //Iterate over all the files inside and copy them
        QDir oldDir(srcInfo.filePath());
        QDir newDir(dstInfo.filePath());
        for (QString file : oldDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
            copyFile(oldDir.filePath(file), newDir.filePath(file), hadUnresolvedConflicts, bytesMoved);
        }
    } else if (srcInfo.isFile()) {
        if (dstInfo.exists()) {
            QSharedPointer<FileConflict> relevantConflict;
            for (QSharedPointer<FileConflict> conflict : fileConflicts) {
                if (conflict->file == file) relevantConflict = conflict;
            }

            if (relevantConflict.isNull()) {
                //Bail out because we've found a conflict that should not be here
                hadUnresolvedConflicts = true;
                return;
            } else {
                switch (relevantConflict->resolution) {
                    case FileConflict::Skip:
                        //Skip this conflict
                        return;
                    case FileConflict::Rename:
                        //Rename the file
                        dstInfo.setFile(relevantConflict->newName);
                        break;
                    case FileConflict::Overwrite:
                        //Do nothing; we'll overwrite the file
                        break;
                }
            }
        }

        //Make the parent directory if it doesn't exist
        if (!dstInfo.dir().exists()) {
            QDir::root().mkpath(dstInfo.dir().path());
        }

        QFile oldFile(srcInfo.filePath());
        QFile newFile(dstInfo.filePath());

        oldFile.open(QFile::ReadOnly);
        newFile.open(QFile::WriteOnly);

        char* data = new char[BLOCK_SIZE];
        while (!oldFile.atEnd()) {
            qint64 dataRead = oldFile.read(data, BLOCK_SIZE);
            newFile.write(data, dataRead);
            bytesMoved += dataRead;

            emit progress(bytesMoved, bytes);
            emit setActionLabelText(tr("Copied %1 out of %2").arg(calculateSize(bytesMoved), calculateSize(bytes)));
        }
        delete[] data;
    } else {
        //We've got a file we can't handle!
        //We should technically never get here!!!!!!!
    }
}
