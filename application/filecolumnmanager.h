/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef FILECOLUMNMANAGER_H
#define FILECOLUMNMANAGER_H

#include <QObject>

class FileColumn;
struct FileColumnManagerPrivate;
class FileColumnManager : public QObject {
        Q_OBJECT
    public:
        explicit FileColumnManager(QObject* parent = nullptr);
        ~FileColumnManager();

        void setCurrent(FileColumn* col);
        FileColumn* current();

    signals:
        void currentChanged();

    private:
        FileColumnManagerPrivate* d;
};

#endif // FILECOLUMNMANAGER_H
