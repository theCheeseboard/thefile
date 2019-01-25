#ifndef TRANSFERENGINE_H
#define TRANSFERENGINE_H

#include <QFileInfo>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QProgressBar>
#include <QDialog>
#include <QScrollArea>
#include <QEventLoop>
#include <QPushButton>
#include <QDirIterator>
#include <QThread>

class TransferPane;

class TransferObject : public QObject
{
    Q_OBJECT
public:
    enum TransferType {
        Copy,
        Move
    };

    explicit TransferObject(QStringList source, QString destination, TransferType type, QObject* parent = NULL);

    QStringList sources();
    QString destination();
    TransferType transferType();

signals:
    void finished();
    void failed();
    void progressUpdate(qulonglong progress, qulonglong total);

private:
    QStringList source;
    QString dest;
    TransferType type;

    qulonglong progress = 0;
    qulonglong total = 0;
};

struct FileConflict
{
    enum Resolution {
        Skip,
        Overwrite,
        Rename
    };

    enum Nature {
        Conflict,
        Warning,
        Error
    };

    QString file;
    QString conflictingFile;
    QString newName;
    QString explanation;
    qulonglong bytes = 0;

    Resolution resolution;
    Nature nature;
};

typedef QList<QSharedPointer<FileConflict>> FileConflictList;
Q_DECLARE_METATYPE(FileConflictList)

class TransferDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TransferDialog(QWidget* parent = NULL);

public slots:
    void addTransferPane(TransferPane* pane);

    void resizeHeight();

private:
    QBoxLayout* transfersLayout;
    QScrollArea* scrollArea;

    QList<TransferPane*> transferPanes;
};

class TransferThread : public QThread
{
    Q_OBJECT

    public:
        explicit TransferThread(QStringList source, QString destination, TransferPane* pane, QObject* parent = nullptr);

    signals:
        void setActionLabelText(QString text);

        void resolveConflicts(FileConflictList conflicts, bool hasNonSimpleConflicts);

        void progress(qulonglong value, qulonglong max);

        void error(QString explanation);

        void done();

    protected:
        void run();
        virtual void doWork(bool& hadUnresolvedConflicts) = 0;

        QStringList source;
        QString destination;
        TransferPane* pane;

        int numberOfFiles = 0;
        qulonglong bytes = 0;

        FileConflictList fileConflicts;
        bool hasNonSimpleConflicts = false;

    private:
        FileConflictList checkConflicts(QString oldFile, QString newFile, qulonglong& bytes);
};

class TransferMove : public TransferThread
{
    Q_OBJECT

    public:
        explicit TransferMove(QStringList source, QString destination, TransferPane* pane, QObject* parent = nullptr);

    protected:
        void doWork(bool& hadUnresolvedConflicts);
};

class TransferCopy : public TransferThread
{
    Q_OBJECT

    public:
        explicit TransferCopy(QStringList source, QString destination, TransferPane* pane, QObject* parent = nullptr);

    protected:
        void doWork(bool& hadUnresolvedConflicts);

        void copyFile(QString file, QString destination, bool& hadUnresolvedConflicts, qulonglong& bytesMoved);
        qulonglong bytesMoved = 0;
};

class TransferEngine : public QObject
{
    Q_OBJECT
public:
    explicit TransferEngine(QObject* parent = NULL);

public slots:
    void addTransfer(TransferObject* transfer);

private:
    QList<TransferObject*> runningTransfers;
    TransferDialog* dialog;

};

#endif // TRANSFERENGINE_H
