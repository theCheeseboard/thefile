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
    this->setShowHidden(false);

    this->setModel(fModel);
    this->setItemDelegate(new FilesystemDelegate);
    this->setRootIndex(fModel->index(directory));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setSelectionMode(ExtendedSelection);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(2, QHeaderView::Fixed);

    errorWidget = new QWidget();
    errorWidget->setVisible(false);
    errorWidget->setParent(this);
    errorWidget->setGeometry(0, 0, this->width(), this->height());
    errorWidget->setAutoFillBackground(true);

    QBoxLayout* errorLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    errorWidget->setLayout(errorLayout);
    errorLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    errorTitleLabel = new QLabel();
    QFont titleFont = errorTitleLabel->font();
    titleFont.setPointSize(20);
    errorTitleLabel->setFont(titleFont);
    errorTitleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    errorLayout->addWidget(errorTitleLabel);

    errorLabel = new QLabel();
    errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    errorLayout->addWidget(errorLabel);

    errorLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

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
        setError("That folder doesn't exist.");
    } else if (!info.isDir()) {
        setError("That's not a folder.");
    } else if (!info.isReadable() && !info.isExecutable()) {
        setError("Couldn't get to that folder.");
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
        /*QMenu* menu = new QMenu();
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

        menu->exec(this->mapToGlobal(pos));*/

        PropertiesDialog* dialog = new PropertiesDialog(info, QCursor::pos());
        dialog->show();
        dialog->setFocus();
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

void FileTable::setShowHidden(bool showHidden) {
    if (showHidden) {
        fModel->setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    } else {
        fModel->setFilter(QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot);
    }
}

bool FileTable::showingHidden() {
    return fModel->filter() & QDir::Hidden;
}

void FileTable::setError(QString error) {
    errorTitleLabel->setText("The bits aren't here!");
    errorLabel->setText(error);
    errorWidget->setVisible(true);
}
