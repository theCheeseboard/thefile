#include "copyworker.h"

copyWorker::copyWorker(QStringList* files, QString d) { // Constructor
    source = files;
    dest = d;
}

copyWorker::~copyWorker() { // Destructor

}

void copyWorker::process() { // Process. Start processing data.
    emit progress(0, 0, "", "");
    qint64 totalBytes = 0;
    qint64 readBytes = 0;

    QStringList *root = new QStringList();
    QStringList *filesToCopy = new QStringList();

    for (QString file : *source) {
        if (QDir(file).exists()) {
            QDirIterator *iterator = new QDirIterator(QDir(file), QDirIterator::Subdirectories);

            while (iterator->hasNext()) {
                QString processFile = iterator->next();
                if (QDir(processFile).exists()) {
                    continue;
                }
                QString pRoot = processFile.left(processFile.lastIndexOf("/"));
                pRoot.remove(file);
                pRoot.remove(0, 1);
                if (pRoot == "") {
                    pRoot = "";
                } else if (pRoot == "." || pRoot == "..") {
                    continue;
                }
                root->append(pRoot);
                filesToCopy->append(processFile);
                totalBytes += QFileInfo(processFile).size();
            }
        } else {
            totalBytes += QFileInfo(file).size();
            root->append("");
            filesToCopy->append(file);
        }
    }

    int i = 0;
    for (QString file : *filesToCopy) {
        QString destination = dest;
        destination.append(root->at(i));
        QDir::root().mkpath(destination);
        QFile src(file);
        QFileInfo info(src);
        QFile d(destination.append("/" + info.fileName()));
        if (info.fileName() == "." || info.fileName() == "..") {
            continue;
        }

        src.open(QFile::ReadOnly);
        d.open(QFile::WriteOnly);

        int emitCounter = 0;

        while (src.bytesAvailable() > 0) {
            if (cancelTransferNow) {
                src.close();
                d.close();
                d.remove();
                emit finished();
                return;
            }
            QByteArray buf = src.read(32768);
            d.write(buf);
            readBytes += buf.length();
            sync();
            //if (emitCounter == 100) {
                emit progress(readBytes, totalBytes, file, dest);
                emitCounter = 0;
            //}
            emitCounter++;
        }
        src.close();
        d.close();
        i++;
    }
    emit finished();
}

void copyWorker::cancelTransfer() {
    cancelTransferNow = true;
}
