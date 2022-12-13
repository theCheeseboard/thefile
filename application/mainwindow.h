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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class FileTab;
struct MainWindowPrivate;
class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

        FileTab *newTab();
        FileTab* newTab(QUrl url);

    private slots:
        void on_actionExit_triggered();

        void on_actionNewTab_triggered();

        void on_actionCloseTab_triggered();

        void on_actionShowHiddenFiles_triggered(bool checked);

        void on_actionGo_triggered();

        void on_actionCopy_triggered();

        void on_actionCut_triggered();

        void on_actionPaste_triggered();

        void on_actionMove_to_Trash_triggered();

        void on_stackedWidget_switchingFrame(int);

        void on_actionSelect_All_triggered();

        void on_actionNew_Window_triggered();

        void on_actionNew_Folder_triggered();

    private:
        Ui::MainWindow* ui;
        MainWindowPrivate* d;

        void updateMenuActions();
};
#endif // MAINWINDOW_H
