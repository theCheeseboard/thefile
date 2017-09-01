#include "driveslist.h"

DrivesList::DrivesList(QWidget *parent) : QListView(parent)
{
    driveModel = new DrivesModel();
    this->setModel(driveModel);
    this->setItemDelegate(new DrivesDelegate);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu(QPoint)));
}

void DrivesList::customContextMenu(QPoint pos) {
    QModelIndex index = this->indexAt(pos);
    PaneItem item = index.data(Qt::UserRole).value<PaneItem>();

    QMenu* menu = new QMenu();
    if (item.itemType() == PaneItem::Disk) {
        menu->addSection("For " + index.data().toString());
        if (item.isMounted()) {
            menu->addAction(QIcon::fromTheme("media-unmount"), "Unmount", [=] {
                QString result = item.attemptUnmount();
                if (result != "") {
                    QMap<QString, QString> actions;
                    actions.insert("force", "Force Unmount");

                    tToast* toast = new tToast();
                    toast->setTitle("Unmount Error");
                    toast->setText(result);
                    toast->setActions(actions);
                    emit this->toast(toast);
                    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                    connect(toast, &tToast::actionClicked, [=](QString key) {
                        if (key == "force") {
                            /*QString fuser;
                            QProcess fuserProc;
                            fuserProc.start("fuser -v " + item.mountPoints().first());
                            fuserProc.waitForFinished();
                            fuser = fuserProc.readAll();*/

                            /*QMessageBox messageBox;
                            messageBox.setWindowTitle("Force Unmount");
                            messageBox.setText("Force unmounting this drive can lead to data loss if another application is currently using it. Do you want to force unmount this drive?");
                            messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                            messageBox.setDefaultButton(QMessageBox::No);
                            messageBox.setIcon(QMessageBox::Warning);
                            if (messageBox.exec() == QMessageBox::Yes) {
                                item.forceUnmount();
                            }*/

                            ForceUnmountDialog dialog(item.mountPoints().first());
                            if (dialog.exec() == ForceUnmountDialog::Accepted) {
                                item.forceUnmount();
                            }
                        }
                    });
                }
            });
        }
    }
    menu->exec(this->mapToGlobal(pos));
}
