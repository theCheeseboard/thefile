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
#include "mainwindow.h"

#include <tsettings.h>
#include <QDir>
#include <tapplication.h>
#include <QCommandLineParser>
#include <QJsonArray>

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);

    if (QDir("/usr/share/thefile/").exists()) {
        a.setShareDir("/usr/share/thefile/");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thefile/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thefile/"));
    }
    a.installTranslators();

    a.setApplicationIcon(QIcon::fromTheme("thefile", QIcon(":/icons/thefile.svg")));
    a.setApplicationVersion("4.0");
    a.setGenericName(QApplication::translate("main", "File Manager"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2021");
    a.setOrganizationName("theSuite");
    a.setApplicationUrl(tApplication::HelpContents, QUrl("https://help.vicr123.com/docs/thefile/intro"));
    a.setApplicationUrl(tApplication::Sources, QUrl("http://github.com/vicr123/theFile"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("http://github.com/vicr123/theFile/issues"));
#ifdef T_BLUEPRINT_BUILD
    a.setApplicationName("theFile Blueprint");
    a.setDesktopFileName("com.vicr123.thefile-blueprint");
#else
    a.setApplicationName("theFile");
    a.setDesktopFileName("com.vicr123.thefile");
#endif

    a.registerCrashTrap();

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/theSuite/theFile/defaults.conf");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(a.translate("main", "folder"), a.translate("main", "Folder to show"), QStringLiteral("[%1]").arg(a.translate("main", "folder")));
    parser.process(a);

    MainWindow* w = new MainWindow();

    QObject::connect(&a, &tApplication::singleInstanceMessage, [ = ](QJsonObject launchMessage) {
        if (launchMessage.contains("files")) {
            MainWindow* w = new MainWindow();

            QJsonArray files = launchMessage.value("files").toArray();
            if (files.isEmpty()) {
                w->newTab();
            } else {
                for (const QJsonValue& file : qAsConst(files)) {
                    w->newTab(QUrl::fromUserInput(file.toString()));
                }
            }

            w->show();
            w->activateWindow();
        }
    });
    QObject::connect(&a, &tApplication::dockIconClicked, [ = ] {
        w->show();
        w->activateWindow();
    });

    QStringList files;
    for (const QString& arg : parser.positionalArguments()) {
        if (QUrl::fromLocalFile(arg).isValid()) {
            files.append(QUrl::fromLocalFile(arg).toEncoded());
        } else {
            files.append(QUrl(arg).toEncoded());
        }
    }
    a.ensureSingleInstance({
        {"files", QJsonArray::fromStringList(files)}
    });
    w->show();

    if (files.isEmpty()) {
        w->newTab();
    } else {
        for (const QString& file : qAsConst(files)) {
            w->newTab(QUrl::fromUserInput(file));
        }
    }

    int retval = a.exec();

    return retval;
}
