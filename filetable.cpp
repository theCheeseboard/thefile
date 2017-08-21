#include "filetable.h"

FileTable::FileTable(QString directory, QWidget *parent) : QTreeView(parent)
{
    fModel = new FilesystemModel();
    fModel->setRootPath(directory);
    fModel->setReadOnly(false);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDragDropMode(DragDrop);
    this->setEditTriggers(SelectedClicked);

    this->setModel(fModel);
    this->setRootIndex(fModel->index(directory));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setSelectionMode(ExtendedSelection);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(3, QHeaderView::Interactive);

    errorWidget = new QWidget();
    errorWidget->setVisible(false);
    errorWidget->setParent(this);
    errorWidget->setGeometry(0, 0, this->width(), this->height());
    errorWidget->setAutoFillBackground(true);

    QBoxLayout* errorLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    errorWidget->setLayout(errorLayout);

    errorLabel = new QLabel();
    errorLayout->addWidget(errorLabel);

    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(activate(QModelIndex)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu(QPoint)));

    vt = (ViewType) settings.value("view/type", List).toInt();
    switch (vt) {
        case List:
            this->setRootIsDecorated(false);
            break;
        case Tree:
            this->setRootIsDecorated(true);
    }
}

QString FileTable::title() {
    return "";
}

void FileTable::activate(QModelIndex index) {
    QFileInfo info = fModel->fileInfo(index);

    if (info.isFile()) {
        QProcess::execute("xdg-open \"" + info.filePath() + "\"");
    } else {
        go(info.filePath());
    }
}

void FileTable::goUp() {
    QFileInfo info = fModel->fileInfo(this->rootIndex());
    if (!info.isRoot()) {
        //this->setRootIndex(this->rootIndex().parent());
        go(info.path());
    }
}

void FileTable::setRootIndex(const QModelIndex &index) {
    QTreeView::setRootIndex(index);
    fModel->setRootPath(fModel->filePath(index));

    this->collapseAll();
    emit titleChanged(fModel->fileName(index));
    emit pathChanged(fModel->filePath(index));
}

void FileTable::setViewType(ViewType type) {
    this->vt = type;

    switch (type) {
        case List:
            this->setRootIsDecorated(false);
            break;
        case Tree:
            this->setRootIsDecorated(true);
    }
}

FileTable::ViewType FileTable::currentViewType() {
    return this->vt;
}

void FileTable::go(QString directory) {
    QModelIndex index = fModel->index(directory);
    this->setRootIndex(index);

    QFileInfo info(directory);
    if (!info.exists()) {
        errorLabel->setText("That folder doesn't exist.");
        errorWidget->setVisible(true);
    } else if (!info.isDir()) {
        errorLabel->setText("That's not a folder.");
        errorWidget->setVisible(true);
    } else if (!info.isReadable() && !info.isExecutable()) {
        errorLabel->setText("Couldn't get to that folder.");
        errorWidget->setVisible(true);
    } else {
        errorWidget->setVisible(false);
    }
}

QString FileTable::path() {
    return fModel->filePath(this->rootIndex());
}

void FileTable::customContextMenu(QPoint pos) {
    QModelIndex index = this->indexAt(pos);
    if (index.isValid()) {
        QFileInfo info = fModel->fileInfo(index);
        QMenu* menu = new QMenu();
        if (info.isFile()) {
            menu->addSection("For file " + info.fileName());
        } else {
            menu->addSection("For folder " + info.fileName());
        }

        menu->addAction(QIcon::fromTheme("edit-rename"), "Rename", [=] {
            this->edit(index);
        });
        menu->addAction(QIcon::fromTheme("edit-delete"), "Delete", [=] {
            tToast* toast = new tToast();
            toast->setTitle("File Deleted");
            toast->setText("Deleted " + info.fileName());

            QMap<QString, QString> actions;
            actions.insert("undo", "Undo");
            toast->setActions(actions);
            toast->setTimeout(5000);
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this);
        });

        menu->exec(this->mapToGlobal(pos));
    }
}

void FileTable::rename() {
    this->edit(currentIndex());
}

void FileTable::rm() {

}

void FileTable::resizeEvent(QResizeEvent *event) {
    errorWidget->resize(this->width(), this->height());
    QTreeView::resizeEvent(event);
}

void FileTable::mkdir(QModelIndex parent) {
    QModelIndex index = fModel->mkdir(parent, "Untitled Folder");
    edit(index);
}
