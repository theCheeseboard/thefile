#include "transferengine.h"

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

        switch (transfer->transferType()) {
            case TransferObject::Copy: {
                //Count files
                pane->setActionLabelText("Counting files...");

                QList<FileConflict> fileConflicts;


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

TransferPane::TransferPane(TransferObject* transfer, QWidget *parent) : QFrame(parent)
{
    this->transfer = transfer;
    this->setFixedWidth(400);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    this->setLayout(layout);

    QLabel* bigTitleLabel = new QLabel();
    QFont bigTitleFont = bigTitleLabel->font();
    bigTitleFont.setPointSize(15);
    bigTitleLabel->setFont(bigTitleFont);
    if (transfer->transferType() == TransferObject::Copy) {
        bigTitleLabel->setText("Copying items");
    } else {
        bigTitleLabel->setText("Moving items");
    }
    layout->addWidget(bigTitleLabel);

    actionLabel = new QLabel();
    layout->addWidget(actionLabel);

    mainWidget = new QWidget();
    layout->addWidget(mainWidget);

    mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    mainLayout->setSpacing(2);

    progress = new QProgressBar();
    progress->setMinimum(0);
    progress->setMaximum(0);
    mainLayout->addWidget(progress);

    mainWidget->setLayout(mainLayout);

    conflictWidget = new QWidget();
    conflictWidget->setVisible(false);
    layout->addWidget(conflictWidget);

    QBoxLayout* fileConflictLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    fileConflictLayout->setSpacing(0);
    conflictWidget->setLayout(fileConflictLayout);

    QPushButton* replaceButton = new QPushButton();
    replaceButton->setText("Replace all files");
    connect(replaceButton, &QPushButton::clicked, [=] {
        FileConflictList newConflicts;
        for (FileConflict conflict : conflicts) {
            conflict.resolution = FileConflict::Overwrite;
            newConflicts.append(conflict);
        }
        emit conflictsResolved(newConflicts);

        mainWidget->setVisible(true);
        conflictWidget->setVisible(false);

        emit heightChanged();
    });
    fileConflictLayout->addWidget(replaceButton);

    QPushButton* skipButton = new QPushButton();
    skipButton->setText("Skip conflicting files");
    fileConflictLayout->addWidget(skipButton);

    QPushButton* moreButton = new QPushButton();
    moreButton->setText("Examine file conflicts");
    fileConflictLayout->addWidget(moreButton);

    this->setFrameShape(QFrame::StyledPanel);
}

void TransferPane::setActionLabelText(QString text) {
    actionLabel->setText(text);
}

void TransferPane::resolveConflicts(QList<FileConflict> conflicts) {
    mainWidget->setVisible(false);
    conflictWidget->setVisible(true);

    if (conflicts.count() == 1) {
        QFileInfo info(conflicts.first().file);
        actionLabel->setText("The file " + info.fileName() + " already exists in the destination.");
    } else {
        actionLabel->setText(QString::number(conflicts.count()) + " file conflicts need to be resolved.");
    }
    this->conflicts = conflicts;
}

void TransferPane::resizeEvent(QResizeEvent *event) {
    emit heightChanged();
    QWidget::resizeEvent(event);
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

    this->show();
    resizeHeight();
}

void TransferDialog::resizeHeight() {
    int totalHeight = 18;
    for (TransferPane* pane : transferPanes) {
        totalHeight += pane->sizeHint().height() + 6;
    }

    if (totalHeight > 300) totalHeight = 300;

    //this->setFixedHeight(totalHeight);
    scrollArea->setFixedHeight(totalHeight);
}

TransferMove::TransferMove(QStringList source, QString destination, TransferPane *pane, QObject *parent) : QThread(parent)
{
    this->source = source;
    this->destination = destination;
    this->pane = pane;

    connect(this, SIGNAL(setActionLabelText(QString)), pane, SLOT(setActionLabelText(QString)));
    connect(this, SIGNAL(resolveConflicts(FileConflictList)), pane, SLOT(resolveConflicts(FileConflictList)));
}

void TransferMove::run() {
    //Count files
    emit setActionLabelText("Counting files...");

    QList<FileConflict> fileConflicts;
    int numberOfFiles = 0;

    for (QString file : source) {
        //Check file conflicts
        QFileInfo info(file);
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
                            FileConflict conflict;
                            conflict.file = iterator.filePath();
                            fileConflicts.append(conflict);
                        }
                    }
                    numberOfFiles++;

                    emit setActionLabelText(QString::number(numberOfFiles) + " files and counting...");
                }
            } else {
                FileConflict conflict;
                conflict.file = file;
                fileConflicts.append(conflict);
            }
        }
        numberOfFiles++;

        emit setActionLabelText(QString::number(numberOfFiles) + " files and counting...");
    }

    if (fileConflicts.count() > 0) {
        QEventLoop loop;
        connect(pane, &TransferPane::conflictsResolved, [=, &fileConflicts, &loop](FileConflictList conflicts) {
            fileConflicts = conflicts;
            loop.exit();
        });
        emit resolveConflicts(fileConflicts);
        loop.exec();
    }

    emit setActionLabelText("Now moving " + QString::number(numberOfFiles) + " files.");

    //Move each file individually to make sure that file conflicts are handled
    int filesMoved;
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
                            for (FileConflict conflict : fileConflicts) {
                                if (conflict.file == file) {
                                    if (conflict.resolution == FileConflict::Overwrite) {
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

                    emit setActionLabelText(QString::number(numberOfFiles) + " files and counting...");
                }
            } else {
                for (FileConflict conflict : fileConflicts) {
                    if (conflict.file == file) {
                        if (conflict.resolution == FileConflict::Overwrite) {
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

        emit setActionLabelText(QString::number(numberOfFiles) + "/" + filesMoved + " files moved...");
    }
}
