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

#include <QCommandLineParser>
#include <QDir>
#include <QJsonArray>
#include <libthefile_global.h>
#include <libthefrisbee_global.h>
#include <plugins/tpluginmanager.h>
#include <tapplication.h>
#include <thefileplugininterface.h>
#include <tsettings.h>
#include <tstylemanager.h>

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);
    a.setApplicationShareDir("thefile");
    a.installTranslators();

    a.addLibraryTranslator(LIBTHEFRISBEE_TRANSLATOR);
    a.addLibraryTranslator(LIBTHEFILE_TRANSLATOR);

    a.setApplicationVersion("4.0.1");
    a.setGenericName(QApplication::translate("main", "File Manager"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2022");
    a.setOrganizationName("theSuite");
    a.setApplicationUrl(tApplication::HelpContents, QUrl("https://help.vicr123.com/docs/thefile/intro"));
    a.setApplicationUrl(tApplication::Sources, QUrl("http://github.com/vicr123/theFile"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("http://github.com/vicr123/theFile/issues"));
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

    a.registerCrashTrap();

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/theSuite/theFile/defaults.conf");

    auto pluginManager = new tPluginManager<PluginInterface>("thefile");
    pluginManager->load();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(a.translate("main", "folder"), a.translate("main", "Folder to show"), QStringLiteral("[%1]").arg(a.translate("main", "folder")));
    parser.process(a);

    tSettings settings;
    QObject::connect(&settings, &tSettings::settingChanged, [=](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

    MainWindow* w = new MainWindow();

    QObject::connect(&a, &tApplication::singleInstanceMessage, [=](QJsonObject launchMessage) {
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
