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
    buttonWidget->setLayout(buttonLayout);

    pathEdit = new QLineEdit();
    pathEdit->setVisible(false);
    connect(pathEdit, SIGNAL(returnPressed()), this, SLOT(pathEditEntered()));

    QPushButton* pathEditButton = new QPushButton;
    pathEditButton->setCheckable(true);
    pathEditButton->setFlat(true);
    pathEditButton->setIcon(QIcon::fromTheme("folder"));
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
    for (QPushButton* button : buttons) {
        button->deleteLater();
    }
    buttons.clear();

    QPushButton* rootButton = new QPushButton();
    rootButton->setText("/");
    connect(rootButton, &QPushButton::clicked, [=] {
        emit go("/");
    });
    buttonLayout->addWidget(rootButton);

    buttons.append(rootButton);

    QStringList parts = path.split("/");
    QString buildParts;
    for (QString part : parts) {
        if (part != "") {
            buildParts += "/" + part;

            QPushButton* button = new QPushButton();
            button->setText(part);
            button->setProperty("goPath", buildParts);
            connect(button, &QPushButton::clicked, [=] {
                emit go(button->property("goPath").toString());
            });
            buttonLayout->addWidget(button);

            buttons.append(button);
        }
    }

    pathEdit->setText(path);
}

void FolderBar::pathEditEntered() {
    emit go(pathEdit->text());
}
