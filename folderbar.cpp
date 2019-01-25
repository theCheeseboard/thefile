#include "folderbar.h"

FolderBar::FolderBar(QWidget *parent) : QWidget(parent)
{
    layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setSpacing(0);
    layout->setMargin(0);
    this->setLayout(layout);

    QWidget* buttonWidget = new QWidget();

    buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);
    buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    buttonWidget->setLayout(buttonLayout);

    pathEdit = new QLineEdit();
    pathEdit->setVisible(false);
    connect(pathEdit, SIGNAL(returnPressed()), this, SLOT(pathEditEntered()));

    QPushButton* pathEditButton = new QPushButton;
    pathEditButton->setCheckable(true);
    pathEditButton->setFlat(true);
    pathEditButton->setIcon(QIcon::fromTheme("folder"));
    pathEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(pathEditButton, &QPushButton::clicked, [=](bool checked) {
        if (checked) {
            pathEdit->setVisible(true);
            buttonWidget->setVisible(false);
        } else {
            pathEdit->setVisible(false);
            buttonWidget->setVisible(true);
        }
    });

    layout->addWidget(pathEditButton);
    layout->addWidget(pathEdit);
    layout->addWidget(buttonWidget);
}

void FolderBar::setPath(QString path) {
    pathEdit->setText(path);

    for (QPushButton* button : buttons) {
        button->deleteLater();
    }
    buttons.clear();

    QStorageInfo sInfo(path);
    QString buildParts;
    int rootButtonWidth = 0;
    if (sInfo.isRoot()) {
        QPushButton* rootButton = new QPushButton();
        rootButton->setText("/");
        connect(rootButton, &QPushButton::clicked, [=] {
            emit go("/");
        });
        buttonLayout->insertWidget(buttonLayout->count() - 1, rootButton);
        rootButtonWidth = rootButton->sizeHint().width();

        buttons.append(rootButton);
    } else {
        path = path.mid(sInfo.rootPath().lastIndexOf("/"));
        buildParts = sInfo.rootPath().left(sInfo.rootPath().lastIndexOf("/"));
    }

    QStringList parts = path.split("/");
    for (QString part : parts) {
        if (part != "") {
            buildParts += "/" + part;

            QPushButton* button = new QPushButton();
            button->setText(part);
            button->setProperty("goPath", buildParts);
            connect(button, &QPushButton::clicked, [=] {
                emit go(button->property("goPath").toString());
            });

            buttons.append(button);
        }
    }

    int remainingWidth = this->width() - rootButtonWidth;
    for (int i = buttons.count() - 1; i >= 0; i--) {
        QPushButton* b = buttons.at(i);
        if (remainingWidth - b->sizeHint().width() >= 0) {
            if (rootButtonWidth == 0 && i == buttons.count() - 1) {
                buttonLayout->insertWidget(buttonLayout->count() - 1, b);
            } else {
                buttonLayout->insertWidget(1, b);
            }
        }
    }
}

void FolderBar::pathEditEntered() {
    emit go(pathEdit->text());
}
