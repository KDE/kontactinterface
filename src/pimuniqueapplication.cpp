/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pimuniqueapplication.h"
using namespace Qt::Literals::StringLiterals;

#include "config-kontactinterface.h"
#include "kontactinterface_debug.h"

#include <KAboutData>
#include <KWindowSystem>

#include "config-kontactinterface.h"
#if KONTACTINTERFACE_HAVE_X11
#include <KStartupInfo>
#include <private/qtx11extras_p.h>
#endif

#if __has_include(<KWaylandExtras>)
#include <KWaylandExtras>
#define HAVE_WAYLAND
#endif

#ifdef Q_OS_WINDOWS
#include <QFont>
#include <Windows.h>
#endif

#include <QCommandLineParser>
#include <QDir>

#include <QMainWindow>
#include <QWidget>

#include <QDBusConnectionInterface>
#include <QDBusInterface>

using namespace KontactInterface;

namespace
{
const char kChromiumFlagsEnv[] = "QTWEBENGINE_CHROMIUM_FLAGS";
const char kDisableInProcessStackTraces[] = "--disable-in-process-stack-traces";

}

//@cond PRIVATE
class Q_DECL_HIDDEN KontactInterface::PimUniqueApplication::PimUniqueApplicationPrivate
{
public:
    PimUniqueApplicationPrivate()
        : cmdArgs(new QCommandLineParser())
    {
    }

    ~PimUniqueApplicationPrivate()
    {
        delete cmdArgs;
    }

    static void disableChromiumCrashHandler()
    {
        // Disable Chromium's own crash handler, which overrides DrKonqi.
        auto flags = qgetenv(kChromiumFlagsEnv);
        if (!flags.contains(kDisableInProcessStackTraces)) {
            qputenv(kChromiumFlagsEnv, QByteArray(flags + " " + kDisableInProcessStackTraces));
        }
    }

    void exportFocusWindow()
    {
#ifdef HAVE_WAYLAND
        KWaylandExtras::self()->exportWindow(QGuiApplication::focusWindow());
#endif
    }

    QCommandLineParser *const cmdArgs;
};
//@endcond

PimUniqueApplication::PimUniqueApplication(int &argc, char **argv[])
    : QApplication(argc, *argv)
    , d(new PimUniqueApplicationPrivate())
{
#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    setStyle(QStringLiteral("breeze"));
    QFont font(QStringLiteral("Segoe UI Emoji"));
    font.setPointSize(10);
    font.setHintingPreference(QFont::PreferNoHinting);
    setFont(font);
#endif

#ifdef HAVE_WAYLAND
    connect(KWaylandExtras::self(), &KWaylandExtras::windowExported, this, [](const auto, const auto &token) {
        qputenv("PINENTRY_GEOM_HINT", QUrl::toPercentEncoding(token));
    });
    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](auto w) {
        if (!w) {
            return;
        }
        d->exportFocusWindow();
    });

    QMetaObject::invokeMethod(
        this,
        [this]() {
            d->exportFocusWindow();
        },
        Qt::QueuedConnection);
#endif
}

PimUniqueApplication::~PimUniqueApplication() = default;

QCommandLineParser *PimUniqueApplication::cmdArgs() const
{
    return d->cmdArgs;
}

void PimUniqueApplication::setAboutData(KAboutData &aboutData)
{
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(d->cmdArgs);
    // This object name is used in start(), and also in kontact's UniqueAppHandler.
    const QString objectName = QLatin1Char('/') + QApplication::applicationName() + "_PimApplication"_L1;
    QDBusConnection::sessionBus().registerObject(objectName,
                                                 this,
                                                 QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableProperties
                                                     | QDBusConnection::ExportAdaptors);
}

static bool callNewInstance(const QString &appName, const QString &serviceName, const QByteArray &asn_id, const QStringList &arguments)
{
    const QString objectName = QLatin1Char('/') + appName + "_PimApplication"_L1;
    QDBusInterface iface(serviceName, objectName, QStringLiteral("org.kde.PIMUniqueApplication"), QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QDBusReply<int> reply = iface.call(QStringLiteral("newInstance"), asn_id, arguments, QDir::currentPath());
        if (reply.isValid()) {
            return true;
        }
    }
    return false;
}

int PimUniqueApplication::newInstance()
{
    return newInstance(QByteArray(), QStringList() << QApplication::applicationName(), QDir::currentPath());
}

bool PimUniqueApplication::start(const QStringList &arguments)
{
    const QString appName = QApplication::applicationName();

    // Try talking to /appName_PimApplication in org.kde.appName,
    // (which could be kontact or the standalone application),
    // otherwise the current app being started will register to DBus.

    const QString serviceName = "org.kde."_L1 + appName;
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName)) {
        QByteArray new_asn_id;
        if (KWindowSystem::isPlatformX11()) {
#if KONTACTINTERFACE_HAVE_X11
            new_asn_id = QX11Info::nextStartupId();
#endif
        } else if (KWindowSystem::isPlatformWayland()) {
            new_asn_id = qgetenv("XDG_ACTIVATION_TOKEN");
        }

        if (callNewInstance(appName, serviceName, new_asn_id, arguments)) {
            return false; // success means that main() can exit now.
        }
    }

    qCDebug(KONTACTINTERFACE_LOG) << "kontact not running -- start standalone application";

    QDBusConnection::sessionBus().registerService(serviceName);

    // Make sure we have DrKonqi
    PimUniqueApplicationPrivate::disableChromiumCrashHandler();

    static_cast<PimUniqueApplication *>(qApp)->activate(arguments, QDir::currentPath());
    return true;
}

// This is called via DBus either by another instance that has just been
// started or by Kontact when the module is activated
int PimUniqueApplication::newInstance(const QByteArray &startupId, const QStringList &arguments, const QString &workingDirectory)
{
    if (KWindowSystem::isPlatformX11()) {
#if KONTACTINTERFACE_HAVE_X11
        KStartupInfo::setStartupId(startupId);
#endif
    } else if (KWindowSystem::isPlatformWayland()) {
        KWindowSystem::setCurrentXdgActivationToken(QString::fromUtf8(startupId));
    }

    const QWidgetList tlws = topLevelWidgets();
    for (QWidget *win : tlws) {
        if (qobject_cast<QMainWindow *>(win)) {
            win->show();
            win->setAttribute(Qt::WA_NativeWindow, true);

            KWindowSystem::activateWindow(win->windowHandle());
            break;
        }
    }

    activate(arguments, workingDirectory);
    return 0;
}

int PimUniqueApplication::activate(const QStringList &arguments, const QString &workingDirectory)
{
    Q_UNUSED(arguments)
    Q_UNUSED(workingDirectory)
    return 0;
}

#include "moc_pimuniqueapplication.cpp"
