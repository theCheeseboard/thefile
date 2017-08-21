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

    QString file;
    QString newName;
    Resolution resolution;
};

typedef QList<FileConflict> FileConflictList;
Q_DECLARE_METATYPE(FileConflictList)

class TransferPane : public QFrame
{
    Q_OBJECT
public:
    explicit TransferPane(TransferObject* transfer, QWidget* parent = NULL);
    //virtual ~TransferPane();

public slots:
    void setActionLabelText(QString text);

    void resolveConflicts(FileConflictList);

signals:
    void heightChanged();

    void cancelTransfer();

    void conflictsResolved(FileConflictList resolutions);

private:
    TransferObject* transfer;

    void resizeEvent(QResizeEvent* event);

    FileConflictList conflicts;

    QWidget *mainWidget, *conflictWidget;
    QBoxLayout *mainLayout;

    QLabel *titleLabel, *actionLabel;
    QProgressBar* progress;
};

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

class TransferMove : public QThread
{
    Q_OBJECT

public:
    explicit TransferMove(QStringList source, QString destination, TransferPane* pane, QObject* parent = NULL);

signals:
    void setActionLabelText(QString text);

    void resolveConflicts(FileConflictList conflicts);

private:
    void run();

    QStringList source;
    QString destination;
    TransferPane* pane;
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
