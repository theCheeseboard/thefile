#ifndef FILETABLE_H
#define FILETABLE_H

#include <QWidget>
#include <QTreeView>
#include <QDir>
#include "filesystemmodel.h"
#include <QHeaderView>
#include <QSettings>
#include <QMenu>
#include <ttoast.h>
#include <QProcess>
#include "propertiesdialog.h"

class FileTable : public QTreeView
{
    Q_OBJECT
public:
    explicit FileTable(QString directory = QDir::homePath(), QWidget *parent = nullptr);

    enum ViewType {
        List,
        Tree
    };

    ViewType currentViewType();

    bool showingHidden();
    void setShowHidden(bool showHidden);

signals:
    void titleChanged(QString title);
    void pathChanged(QString path);

public slots:
    QString title();

    QString path();

    void goUp();

    void go(QString directory);

    void setViewType(ViewType type);

    void rename();

    void rm();

    void mkdir(QModelIndex parent);

private slots:
    void activate(QModelIndex index);

    void setRootIndex(const QModelIndex &index) override;

    void customContextMenu(QPoint pos);

    void setError(QString error);

private:
    FilesystemModel* fModel;
    QWidget* errorWidget;
    QLabel *errorTitleLabel, *errorLabel;

    ViewType vt;

    void resizeEvent(QResizeEvent* event) override;

    QSettings settings;
};

#endif // FILETABLE_H
