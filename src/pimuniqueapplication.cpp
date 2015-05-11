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

#include "config-kontactinterface.h"
#include "pimuniqueapplication.h"
#include "kontactinterface_debug.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KStartupInfo>
#include <KWindowSystem>

#include <QCommandLineParser>
#include <QLoggingCategory>

#include <qdbusconnectioninterface.h>
#include <qdbusinterface.h>
#include <qdatastream.h>

using namespace KontactInterface;

//@cond PRIVATE
class KontactInterface::PimUniqueApplication::Private
{
};
//@endcond

PimUniqueApplication::PimUniqueApplication()
    : QApplication()
    , d(new Private())
{
    // This object name is used in start(), and also in kontact's UniqueAppHandler.
    const QString objectName = QLatin1Char('/') + applicationName() + QLatin1String("_PimApplication");
    QDBusConnection::sessionBus().registerObject(
        objectName, this,
        QDBusConnection::ExportScriptableSlots |
        QDBusConnection::ExportScriptableProperties |
        QDBusConnection::ExportAdaptors);
}

PimUniqueApplication::~PimUniqueApplication()
{
    delete d;
}

bool PimUniqueApplication::start(bool allowNonUnique)
{
    const QString appName = QApplication::applicationName();
    // Try talking to /appName_PimApplication in org.kde.appName,
    // (which could be kontact or the standalone application),
    // otherwise fall back to starting a new app

    const QString serviceName = QLatin1String("org.kde.") + appName;
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName)) {
        QByteArray new_asn_id;
#if KONTACTINTERFACE_HAVE_X11
        KStartupInfoId id;
        if (kapp) {   // KApplication constructor unsets the env. variable
            id.initId(kapp->startupId());
        } else {
            id = KStartupInfo::currentStartupIdEnv();
        }
        if (!id.isNull()) {
            new_asn_id = id.id();
        }
#endif

        //KWindowSystem::allowExternalProcessWindowActivation();

        const QString objectName = QLatin1Char('/') + appName + QLatin1String("_PimApplication");
        qCDebug(KONTACTINTERFACE_LOG) << objectName;
        QDBusInterface iface(serviceName,
                             objectName,
                             QLatin1String("org.kde.PIMUniqueApplication"),
                             QDBusConnection::sessionBus());

        QDBusInterface iface(serviceName, objectName,
                             QLatin1String("org.kde.PIMUniqueApplication"),
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<int> reply = iface.call(QLatin1String("newInstance"),
                                               new_asn_id,
                                               QCoreApplication::arguments());
            if (reply.isValid()) {
                return false; // success means that main() can exist now.
            }
        }
    }

    qCDebug(KONTACTINTERFACE_LOG) << "kontact not running -- start standalone application";
    // kontact not running -- start standalone application.
    //return KUniqueApplication::start(flags);
    newInstance();
    return true;
}

int PimUniqueApplication::newInstance(const QStringList &arguments)
{
    // nothing to do
    return 0;
}
