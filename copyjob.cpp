#include "copyjob.h"

extern RunningTransfers* TransferWin;

extern QString calculateSize(quint64 size);

CopyJob::CopyJob(QStringList source, QString destination)
{
    this->source = source;
    this->destination = destination;

    frame = new QFrame();
    frame->setParent(TransferWin);
    QBoxLayout* frameLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    frame->setLayout(frameLayout);
    frame->setAutoFillBackground(true);
    frame->moveToThread(TransferWin->thread());

    filesLabel = new QLabel();
    filesLabel->setParent(TransferWin);
    filesLabel->setText("Waiting...");
    frameLayout->addWidget(filesLabel);
    filesLabel->moveToThread(TransferWin->thread());

    QLabel* destLabel = new QLabel();
    destLabel->setParent(TransferWin);
    destLabel->setText("-> " + destination);
    frameLayout->addWidget(destLabel);
    frameLayout->moveToThread(TransferWin->thread());

    bar = new QProgressBar();
    bar->setParent(TransferWin);
    frameLayout->addWidget(bar);
    bar->moveToThread(TransferWin->thread());

    TransferWin->AddFrame(frame);

    copyThread = new QThread();
    CopyJobWorker* worker = new CopyJobWorker(source, destination);
    worker->moveToThread(copyThread);
    connect(copyThread, SIGNAL(finished()), copyThread, SLOT(deleteLater()));
    connect(copyThread, &QThread::destroyed, [=]() {
        TransferWin->RemoveFrame(frame);
        frame->deleteLater();
    });
    connect(copyThread, SIGNAL(started()), worker, SLOT(startCopy()));
    connect(worker, SIGNAL(progressChanged(qulonglong,qulonglong)), this, SLOT(progressChanged(qulonglong,qulonglong)));
    connect(worker, SIGNAL(statusTextChanged(QString)), this, SLOT(statusTextChanged(QString)));
    this->moveToThread(copyThread);

    copyThread->start();

    connect(worker, SIGNAL(finishedCopy()), copyThread, SLOT(quit()));
}

void CopyJob::progressChanged(qulonglong bytesCopied, qulonglong bytesToCopy) {
    bar->setMaximum(bytesToCopy);
    bar->setValue(bytesCopied);
}

void CopyJob::statusTextChanged(QString statusText) {
    QFontMetrics metrics(filesLabel->font());
    filesLabel->setText(metrics.elidedText(statusText, Qt::ElideMiddle, filesLabel->width()));
}

CopyJobWorker::CopyJobWorker(QStringList source, QString destination) {
    this->source = source;
    this->destination = destination;
}

void CopyJobWorker::startCopy() {
    qDebug() << "Copy Started to " + destination;
    emit progressChanged(0, 0);
    emit statusTextChanged("Counting files...");

    for (QString file : source) {
        QString localFile = QUrl(file).toLocalFile();
        if (QDir(localFile).exists()) {
            qDebug() << "Counting directory " << file;
            QDirIterator iterator(QDir(localFile), QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                iterator.next();
                if (QFile(iterator.fileName()).exists()) {
                    totalBytes += QFile(iterator.fileName()).size();
                    emit statusTextChanged("Counting files (" + calculateSize(totalBytes) + " so far.)");
                    //QThread::msleep(100);
                }
            }
        } else {
            totalBytes += QFile(localFile).size();
        }

        emit statusTextChanged("Counting files (" + calculateSize(totalBytes) + " so far.)");
    }

    emit statusTextChanged("Copying files. " + calculateSize(totalBytes) + " to copy.");

    bool ignoreError = false;

    for (QString file : source) {
        QString localFile = QUrl(file).toLocalFile();

        if (QDir(localFile).exists()) {
            QDir directory(localFile);
            qDebug() << "Copying directory " << file;
            QDirIterator iterator(directory, QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                QString filePath = iterator.next();
                if (iterator.fileName() != "." && iterator.fileName() != "..") {
                    if (QFile(filePath).exists() && !QDir(filePath).exists()) {
                        copyFile(filePath, destination + "/" + directory.dirName() + filePath.remove(localFile));
                    }
                }
            }
        } else {
            copyFile(QUrl(file).toLocalFile(), destination);
        }
    }

    while (ErrorFiles.count() != 0) {
        emit statusTextChanged("Some files failed to copy.");
        thread()->sleep(5);
        break;
    }

    emit finishedCopy();
}

void CopyJobWorker::copyFile(QString source, QString destination) {
    qDebug() << "Copy " + source + " -> " + destination;
    QDir::root().mkpath(QFileInfo(destination).absoluteDir().absolutePath());

    if (QDir(destination).exists()) {
        destination = destination + source.mid(source.lastIndexOf("/"));
    }

    QFile sourceFile(source);
    QFile destFile(destination);
    if (!sourceFile.open(QFile::ReadOnly) || !destFile.open(QFile::WriteOnly)) {
        sourceFile.close();
        destFile.close();
        ErrorFiles.append(source);
        return;
    }

    qlonglong runningSize = 0;
    while (runningSize < sourceFile.size()) {
        QByteArray buf = sourceFile.read(4194304); //Read 4 MiB into a buffer
        destFile.write(buf); //Write into destination file
        destFile.flush(); //Flush destination file
        runningSize += buf.length();
        copiedBytes += buf.length();

        emit progressChanged(copiedBytes, totalBytes);
    }

    sourceFile.close();
    destFile.close();
}
