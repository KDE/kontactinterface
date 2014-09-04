/* This file is part of the KDE project

   Copyright 2008 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KONTACTINTERFACE_PIMUNIQUEAPPLICATION_H
#define KONTACTINTERFACE_PIMUNIQUEAPPLICATION_H

#include "kontactinterface_export.h"
#include <kuniqueapplication.h>

namespace KontactInterface
{

/**
 * KDEPIM applications which can be integrated into kontact should use
 * PimUniqueApplication instead of KUniqueApplication.
 * This makes command-line handling work, i.e. you can launch "korganizer"
 * and if kontact is already running, it will load the korganizer part and
 * switch to it.
 */
class KONTACTINTERFACE_EXPORT PimUniqueApplication : public KUniqueApplication
{
public:
    explicit PimUniqueApplication();
    ~PimUniqueApplication();

    /**
     * @see KUniqueApplication::start
     */
    static bool start();

    /**
     * @see KUniqueApplication::start
     *
     * @param flags the application start flags
     * @since 4.5
     */
    static bool start(KUniqueApplication::StartFlags flags);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
