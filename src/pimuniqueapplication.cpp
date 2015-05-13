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

#include <KStartupInfo>
#include <KWindowSystem>
#include <KAboutData>

#include <QCommandLineParser>
#include <QLoggingCategory>

#include <QWidget>
#include <QMainWindow>

#include <QDBusInterface>
#include <QDBusConnectionInterface>

using namespace KontactInterface;

//@cond PRIVATE
class KontactInterface::PimUniqueApplication::Private
{
public:
    Private()
        : cmdArgs(Q_NULLPTR)
    {}

    ~Private()
    {
        delete cmdArgs;
    }

    QCommandLineParser *cmdArgs;
};
//@endcond

PimUniqueApplication::PimUniqueApplication(int &argc, char **argv[],
                                           KAboutData &aboutData)
    : QApplication(argc, *argv)
    , d(new Private())
{
    d->cmdArgs = new QCommandLineParser();

    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(d->cmdArgs);

    // This object name is used in start(), and also in kontact's UniqueAppHandler.
    const QString objectName = QLatin1Char('/') + QApplication::applicationName() + QLatin1String("_PimApplication");
    QDBusConnection::sessionBus().registerObject(
        objectName, this,
        QDBusConnection::ExportAllSlots |
        QDBusConnection::ExportScriptableProperties |
        QDBusConnection::ExportAdaptors);

}

PimUniqueApplication::~PimUniqueApplication()
{
    delete d;
}

QCommandLineParser* PimUniqueApplication::cmdArgs() const
{
    return d->cmdArgs;
}

bool PimUniqueApplication::start(const QStringList &arguments, bool unique)
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
        if (!KStartupInfo::startupId().isEmpty()) {
            id.initId(KStartupInfo::startupId());
        } else {
            id = KStartupInfo::currentStartupIdEnv();
        }
        if (!id.isNull()) {
            new_asn_id = id.id();
        }
#endif

        KWindowSystem::allowExternalProcessWindowActivation();

        const QString objectName = QLatin1Char('/') + appName + QLatin1String("_PimApplication");
        qCDebug(KONTACTINTERFACE_LOG) << objectName;
        QDBusInterface iface(serviceName,
                             objectName,
                             QLatin1String("org.kde.PIMUniqueApplication"),
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<int> reply = iface.call(QLatin1String("newInstance"),
                                               new_asn_id,
                                               arguments);
            if (reply.isValid()) {
                return false; // success means that main() can exist now.
            }
        }
    }

    qCDebug(KONTACTINTERFACE_LOG) << "kontact not running -- start standalone application";

    if (unique) {
        QDBusConnection::sessionBus().registerService(serviceName);
    }

    static_cast<PimUniqueApplication*>(qApp)->activate(arguments);
    return true;
}

// This is called via DBus either by another instance that has just been
// started or by Kontact when the module is activated
int PimUniqueApplication::newInstance(const QByteArray &startupId,
                                      const QStringList &arguments)
{
    KStartupInfo::setStartupId(startupId);

    const QWidgetList tlws = topLevelWidgets();
    for (QWidget *win : tlws) {
        if (qobject_cast<QMainWindow*>(win)) {
            win->show();
            KWindowSystem::forceActiveWindow(win->winId());
            KStartupInfo::setNewStartupId(win, startupId);
            break;
        }
    }

    activate(arguments);
    return 0;
}


int PimUniqueApplication::activate(const QStringList &arguments)
{
    Q_UNUSED(arguments);
    return 0;
}
