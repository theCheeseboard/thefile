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
#include "plugin.h"

#include "directories/idevicedirectoryhandler.h"
#include "idevicewatcher.h"
#include "sidebar/idevicesidebarsectionfactory.h"
#include <QDebug>
#include <QIcon>
#include <resourcemanager.h>
#include <sidebarmanager.h>
#include <tapplication.h>
#include <tlogger.h>

struct PluginPrivate {
        IDeviceWatcher* watcher;
};

Plugin::Plugin() {
    d = new PluginPrivate();
}

Plugin::~Plugin() {
    delete d;
}

void Plugin::activate() {
    tDebug("IDevicePlugin") << "IDevicePlugin loaded";
    d->watcher = new IDeviceWatcher();
    ResourceManager::instance()->registerDirectoryHandler(new IDeviceDirectoryHandler(d->watcher));
    SidebarManager::registerSidebarFactory(new IDeviceSidebarSectionFactory(d->watcher));
}

void Plugin::deactivate() {
}

