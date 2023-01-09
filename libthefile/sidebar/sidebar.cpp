/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "sidebar.h"
#include "ui_sidebar.h"

#include "devicesmodel.h"
#include "sidebarmanager.h"
#include "sidebarsectionfactory.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/encryptedinterface.h>
#include <DriveObjects/filesysteminterface.h>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QStandardPaths>
#include <QUrl>
#include <directory.h>
#include <diskoperationmanager.h>
#include <driveobjectmanager.h>
#include <frisbeeexception.h>
#include <resourcemanager.h>
#include <tjobmanager.h>
#include <tlogger.h>
#include <tmessagebox.h>
#include <tpaintcalculator.h>
#include <tpopover.h>

struct SidebarPrivate {
};

Sidebar::Sidebar(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Sidebar) {
    ui->setupUi(this);
    d = new SidebarPrivate();

    for (auto factory : SidebarManager::factories()) {
        for (auto sidebar : factory->init()) {
            QLabel* label = new QLabel(this);
            label->setText(sidebar->label().toUpper());
            label->setMargin(9);

            auto font = label->font();
            font.setBold(true);
            label->setFont(font);

            connect(sidebar, &SidebarSection::labelChanged, this, [label, sidebar] {
                label->setText(sidebar->label());
            });
            connect(sidebar, &SidebarSection::visibilityChanged, this, [label, sidebar] {
                label->setVisible(sidebar->visible());
                sidebar->widget()->setVisible(sidebar->visible());
            });
            connect(sidebar, &SidebarSection::navigate, this, &Sidebar::navigate);
            connect(sidebar, &SidebarSection::copyFiles, this, &Sidebar::copyFiles);
            connect(sidebar, &SidebarSection::moveFiles, this, &Sidebar::moveFiles);

            label->setVisible(sidebar->visible());
            sidebar->widget()->setVisible(sidebar->visible());

            ui->sidebarContainer->addWidget(label);
            ui->sidebarContainer->addWidget(sidebar->widget());

            ui->sidebarContainer->setAlignment(label, Qt::AlignLeft | Qt::AlignVCenter);

            if (auto list = qobject_cast<QListView*>(sidebar->widget())) {
                list->setItemDelegate(new SidebarDelegate());
                list->setFrameShape(QFrame::NoFrame);
            }
        }
    }
}

Sidebar::~Sidebar() {
    delete d;
    delete ui;
}

SidebarDelegate::SidebarDelegate(QObject* parent) :
    QStyledItemDelegate(parent) {
}

void SidebarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
    painter->restore();

    tPaintCalculator calculator;
    calculator.setLayoutDirection(option.direction);
    calculator.setPainter(painter);
    calculator.setDrawBounds(option.rect);

    painter->setPen(Qt::transparent);

    QPen textPen;
    if (option.state & QStyle::State_Selected) {
        textPen = option.palette.color(QPalette::HighlightedText);
    } else if (option.state & QStyle::State_MouseOver) {
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        textPen = option.palette.color(QPalette::WindowText);
    }

    QRect contentRect = option.rect.adjusted(SC_DPI(9), 0, 0, 0);
    QRect iconRect = contentRect, textRect = contentRect;

    textRect.setHeight(contentRect.height() - SC_DPI(2));

    if (!index.data(Qt::DecorationRole).value<QIcon>().isNull()) {
        QSize iconSize = ((QAbstractItemView*) option.widget)->iconSize();
        if (!iconSize.isValid()) {
            iconSize = QSize(SC_DPI(16), SC_DPI(16));
        }

        iconRect.setSize(iconSize);
        QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        QImage iconImage = icon.pixmap(iconSize).toImage();
        iconRect.moveLeft(contentRect.left() + SC_DPI(2));
        iconRect.moveTop(contentRect.top() + (contentRect.height() / 2) - (iconRect.height() / 2));

        textRect.setLeft(iconRect.right() + SC_DPI(6));

        calculator.addRect(iconRect, [painter, iconImage](QRectF drawBounds) {
            painter->drawImage(drawBounds, iconImage);
        });
    } else {
        textRect.setLeft(contentRect.left() + SC_DPI(6));
    }

    QString text = index.data(Qt::DisplayRole).toString();
    calculator.addRect(textRect, [painter, textPen, option, text](QRectF drawBounds) {
        painter->setPen(textPen);
        painter->setFont(option.font);
        painter->drawText(drawBounds, Qt::AlignLeft | Qt::AlignVCenter, text);
    });

    if (index.data(DevicesModel::MountedRole).toBool()) {
        // Draw the mounted indicator
        QRect mountIndicator = option.rect;
        mountIndicator.setWidth(SC_DPI(6));

        calculator.addRect(mountIndicator, [painter](QRectF drawBounds) {
            painter->setPen(Qt::transparent);
            painter->setBrush(QColor(0, 200, 0));
            painter->drawRect(drawBounds);
        });
    }

    calculator.performPaint();
}
