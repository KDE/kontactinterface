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
#include <QDir>

#include <QWidget>
#include <QMainWindow>

#include <QDBusInterface>
#include <QDBusConnectionInterface>

using namespace KontactInterface;

namespace {

const char kChromiumFlagsEnv[] = "QTWEBENGINE_CHROMIUM_FLAGS";
const char kDisableInProcessStackTraces[] = "--disable-in-process-stack-traces";

}

//@cond PRIVATE
class Q_DECL_HIDDEN KontactInterface::PimUniqueApplication::Private
{
public:
    Private()
        : cmdArgs(new QCommandLineParser())
    {}

    ~Private()
    {
        delete cmdArgs;
    }

    static void disableChromiumCrashHandler()
    {
        // Disable Chromium's own crash handler, which overrides DrKonqi.
        auto flags = qgetenv(kChromiumFlagsEnv);
        if (!flags.contains(kDisableInProcessStackTraces)) {
            qputenv(kChromiumFlagsEnv, flags + " " + kDisableInProcessStackTraces);
        }
    }

    QCommandLineParser *const cmdArgs;
};
//@endcond

PimUniqueApplication::PimUniqueApplication(int &argc, char **argv[])
    : QApplication(argc, *argv)
    , d(new Private())
{
}

PimUniqueApplication::~PimUniqueApplication()
{
    delete d;
}

QCommandLineParser *PimUniqueApplication::cmdArgs() const
{
    return d->cmdArgs;
}

void PimUniqueApplication::setAboutData(KAboutData &aboutData)
{
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(d->cmdArgs);
    // This object name is used in start(), and also in kontact's UniqueAppHandler.
    const QString objectName = QLatin1Char('/') + QApplication::applicationName() + QLatin1String("_PimApplication");
    QDBusConnection::sessionBus().registerObject(
        objectName, this,
        QDBusConnection::ExportScriptableSlots |
        QDBusConnection::ExportScriptableProperties |
        QDBusConnection::ExportAdaptors);
}

static bool callNewInstance(const QString &appName, const QString &serviceName, const QByteArray &asn_id, const QStringList &arguments)
{
    const QString objectName = QLatin1Char('/') + appName + QLatin1String("_PimApplication");
    QDBusInterface iface(serviceName,
            objectName,
            QStringLiteral("org.kde.PIMUniqueApplication"),
            QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QDBusReply<int> reply = iface.call(QStringLiteral("newInstance"),
                asn_id,
                arguments,
                QDir::currentPath());
        if (reply.isValid()) {
            return true;
        }
    }
    return false;
}

int PimUniqueApplication::newInstance()
{
    return newInstance(KStartupInfo::startupId(), QStringList() << QApplication::applicationName(), QDir::currentPath());
}


bool PimUniqueApplication::start(const QStringList &arguments)
{
    const QString appName = QApplication::applicationName();

    // Try talking to /appName_PimApplication in org.kde.appName,
    // (which could be kontact or the standalone application),
    // otherwise the current app being started will register to DBus.

    qDebug() << "bool PimUniqueApplication::start(const QStringList &arguments)  " << arguments;
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

        if (callNewInstance(appName, serviceName, new_asn_id, arguments)) {
            return false;  // success means that main() can exit now.
        }
    }

    qCDebug(KONTACTINTERFACE_LOG) << "kontact not running -- start standalone application";

    QDBusConnection::sessionBus().registerService(serviceName);

    // Make sure we have DrKonqi
    Private::disableChromiumCrashHandler();

    static_cast<PimUniqueApplication *>(qApp)->activate(arguments, QDir::currentPath());
    return true;
}

bool PimUniqueApplication::activateApplication(const QString &appName, const QStringList &additionalArguments)
{
    const QString serviceName = QLatin1String("org.kde.") + appName;
    QStringList arguments{ appName };
    arguments += additionalArguments;
    // Start it standalone if not already running (if kontact is running, then this will do nothing)
    QDBusConnection::sessionBus().interface()->startService(serviceName);
    return callNewInstance(appName, serviceName, KStartupInfo::createNewStartupId(), arguments);

}

// This is called via DBus either by another instance that has just been
// started or by Kontact when the module is activated
int PimUniqueApplication::newInstance(const QByteArray &startupId,
                                      const QStringList &arguments,
                                      const QString &workingDirectory)
{
    KStartupInfo::setStartupId(startupId);

    const QWidgetList tlws = topLevelWidgets();
    for (QWidget *win : tlws) {
        if (qobject_cast<QMainWindow *>(win)) {
            win->show();
            win->setAttribute(Qt::WA_NativeWindow, true);
            KStartupInfo::setNewStartupId(win->windowHandle(), startupId); // this moves 'win' to the current desktop
#ifdef Q_OS_WIN
            KWindowSystem::forceActiveWindow(win->winId());
#endif
            break;
        }
    }

    activate(arguments, workingDirectory);
    return 0;
}

int PimUniqueApplication::activate(const QStringList &arguments, const QString &workingDirectory)
{
    Q_UNUSED(arguments);
    Q_UNUSED(workingDirectory);
    return 0;
}

