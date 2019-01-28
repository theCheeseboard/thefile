#include "filetable.h"

#include <QScroller>
#include <QGestureEvent>
#include <tpropertyanimation.h>

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
    this->setAttribute(Qt::WA_AcceptTouchEvents);

    this->setModel(fModel);
    this->setItemDelegate(new FilesystemDelegate);
    this->setRootIndex(fModel->index(directory));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setSelectionMode(ExtendedSelection);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(2, QHeaderView::Fixed);

    this->viewport()->installEventFilter(this);

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

    selectionPopup = new SelectionPopup(this);
    selectionPopup->setGeometry(this->width() / 2 - selectionPopup->width() / 2, this->height() + 9, selectionPopup->width(), selectionPopup->height());
    selectionPopup->show();
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=] {
        this->updateSelectionPopup();
    });
    connect(selectionPopup, &SelectionPopup::clearSelection, [=] {
        this->selectionModel()->clear();
    });

    QScroller::grabGesture(this->viewport(), QScroller::TouchGesture);
    this->viewport()->grabGesture(Qt::TapGesture, Qt::DontStartGestureOnChildren);
    this->viewport()->grabGesture(Qt::TapAndHoldGesture, Qt::DontStartGestureOnChildren);
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
        setError(tr("That folder doesn't exist."));
    } else if (!info.isDir()) {
        setError(tr("That's not a folder."));
    } else if (!info.isReadable() && !info.isExecutable()) {
        setError(tr("Couldn't get to that folder."));
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
        QList<QFileInfo> info;

        for (QModelIndex i : this->selectionModel()->selectedIndexes()) {
            if (i.column() == 0) {
                info.append(fModel->fileInfo(i));
            }
        }

        PropertiesDialog* dialog = new PropertiesDialog(info, info.first().dir(), QCursor::pos());
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

    if (this->selectionModel()->selectedRows().isEmpty()) {
        selectionPopup->setGeometry(this->width() / 2 - selectionPopup->width() / 2, this->height() + 9, selectionPopup->width(), selectionPopup->height());
    } else {
        selectionPopup->setGeometry(this->width() / 2 - selectionPopup->width() / 2, this->height() - selectionPopup->height() - 9, selectionPopup->width(), selectionPopup->height());
    }

    QTreeView::resizeEvent(event);
}

void FileTable::mkdir(QModelIndex parent) {
    QModelIndex index = fModel->mkdir(parent, tr("Untitled Folder"));
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
    errorTitleLabel->setText(tr("The bits aren't here!"));
    errorLabel->setText(error);
    errorWidget->setVisible(true);
}

bool FileTable::eventFilter(QObject* object, QEvent *event) {
    if (object == this->viewport()) {
        if (event->type() == QEvent::TouchBegin) {
            QTouchEvent* e = (QTouchEvent*) event;
            //Synthesize a mouse enter event

            //QApplication::sendEvent(this, new QMouseEvent(QMouseEvent::MouseButtonPress, e->touchPoints().first().pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier));
            QApplication::sendEvent(object, new QEnterEvent(e->touchPoints().first().pos(), this->viewport()->mapTo(this->window(), e->touchPoints().first().pos().toPoint()),
                                                            this->viewport()->mapToGlobal(e->touchPoints().first().pos().toPoint())));

            event->accept();
        } else if (event->type() == QEvent::TouchEnd) {
            QApplication::sendEvent(object, new QEvent(QEvent::Leave));
            event->accept();
        } else if (event->type() == QEvent::TouchCancel) {
            QApplication::sendEvent(object, new QEvent(QEvent::Leave));
            event->accept();
        } else if (event->type() == QEvent::Gesture) {
            QGestureEvent* e = (QGestureEvent*) event;

            e->accept(Qt::TapGesture);
            e->accept(Qt::TapAndHoldGesture);

            QTapGesture* tapGesture = (QTapGesture*) e->gesture(Qt::TapGesture);
            QTapAndHoldGesture* tapHoldGesture = (QTapAndHoldGesture*) e->gesture(Qt::TapAndHoldGesture);
            if (tapGesture) {
                if (tapGesture->state() == Qt::GestureFinished && tapHoldState != Qt::GestureFinished) {
                    QModelIndex activator = this->indexAt(tapGesture->position().toPoint());
                    if (activator.isValid()) {
                        if (this->selectionModel()->selectedRows().count() == 0) {
                            this->activate(activator);
                        } else {
                            this->selectionModel()->select(activator, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
                        }
                    }
                    this->updateSelectionPopup();
                } else if (tapGesture->state() == Qt::GestureStarted) {
                    tapHoldState = Qt::NoGesture;
                }
            }

            if (tapHoldGesture) {
                if (tapHoldGesture->state() == Qt::GestureFinished) {
                    QModelIndex selection = this->indexAt(this->viewport()->mapFrom(this->window(), tapHoldGesture->position().toPoint()) - QPoint(0, this->rowHeight(fModel->index(0, 0))));
                    this->selectionModel()->select(selection, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
                    this->updateSelectionPopup();
                }

                tapHoldState = tapHoldGesture->state();
            }
        } else {
            return QTreeView::eventFilter(object, event);
        }
    }
    return true;
}

bool FileTable::event(QEvent *event) {
    return QTreeView::event(event);
}

void FileTable::updateSelectionPopup() {
    QModelIndexList selection = this->selectionModel()->selectedRows();

    tPropertyAnimation* anim = new tPropertyAnimation(this->selectionPopup, "geometry");
    anim->setStartValue(this->selectionPopup->geometry());
    if (selection.isEmpty()) {
        anim->setEndValue(QRect(this->width() / 2 - selectionPopup->width() / 2, this->height() + 9, selectionPopup->width(), selectionPopup->height()));
    } else {
        anim->setEndValue(QRect(this->width() / 2 - selectionPopup->width() / 2, this->height() - selectionPopup->height() - 9, selectionPopup->width(), selectionPopup->height()));
    }
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    anim->start();

    QList<QFileInfo> info;
    for (QModelIndex i : this->selectionModel()->selectedIndexes()) {
        if (i.column() == 0) {
            info.append(fModel->fileInfo(i));
        }
    }
    selectionPopup->setSelection(info);
}
