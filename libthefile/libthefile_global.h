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
#ifndef LIBTHEFILE_GLOBAL_H
#define LIBTHEFILE_GLOBAL_H

#include <QEnableSharedFromThis>
#include <QtCore/qglobal.h>

#if defined(LIBTHEFILE_LIBRARY)
    #define LIBTHEFILE_EXPORT Q_DECL_EXPORT
#else
    #define LIBTHEFILE_EXPORT Q_DECL_IMPORT
#endif

#define LIBTHEFILE_TRANSLATOR "thefile/libthefile"

template<typename T> class tfSharedFromThis : public QEnableSharedFromThis<T> {
    public:
        QSharedPointer<T> sharedFromThis() {
            auto ptr = QEnableSharedFromThis<T>::sharedFromThis();
            if (ptr) return ptr;
            return QSharedPointer<T>(static_cast<T*>(this));
        }
};

#endif // LIBTHEFILE_GLOBAL_H
