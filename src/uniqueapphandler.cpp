/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2003, 2008 David Faure <faure@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uniqueapphandler.h"
#include "core.h"

#include "processes.h"

#include "kontactinterface_debug.h"
#include <KStartupInfo>
#include <kwindowsystem.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QCommandLineParser>

#ifdef Q_OS_WIN
#include <process.h>
#endif

/*
 Test plan for the various cases of interaction between standalone apps and kontact:

 1) start kontact, select "Mail".
 1a) type "korganizer" -> it switches to korganizer
 1b) type "kmail" -> it switches to kmail
 1c) type "kaddressbook" -> it switches to kaddressbook
 1d) type "kmail foo@kde.org" -> it opens a kmail composer, without switching
 1e) type "knode" -> it switches to knode [unless configured to be external]
 1f) type "kaddressbook --new-contact" -> it opens a kaddressbook contact window

 2) close kontact. Launch kmail. Launch kontact again.
 2a) click "Mail" icon -> kontact doesn't load a part, but activates the kmail window
 2b) type "kmail foo@kde.org" -> standalone kmail opens composer.
 2c) close kmail, click "Mail" icon -> kontact loads the kmail part.
 2d) type "kmail" -> kontact is brought to front

 3) close kontact. Launch korganizer, then kontact.
 3a) both Todo and Calendar activate the running korganizer.
 3b) type "korganizer" -> standalone korganizer is brought to front
 3c) close korganizer, click Calendar or Todo -> kontact loads part.
 3d) type "korganizer" -> kontact is brought to front

 4) close kontact. Launch kaddressbook, then kontact.
 4a) "Contacts" icon activate the running kaddressbook.
 4b) type "kaddressbook" -> standalone kaddressbook is brought to front
 4c) close kaddressbook, type "kaddressbook -a foo@kde.org" -> kontact loads part and opens editor
 4d) type "kaddressbook" -> kontact is brought to front

 5) start "kontact --module summaryplugin"
 5a) type "qdbus org.kde.kmail /kmail_PimApplication newInstance '' ''" ->
     kontact switches to kmail (#103775)
 5b) type "kmail" -> kontact is brought to front
 5c) type "kontact" -> kontact is brought to front
 5d) type "kontact --module summaryplugin" -> kontact switches to summary

*/

using namespace KontactInterface;

//@cond PRIVATE
class UniqueAppHandler::UniqueAppHandlerPrivate
{
public:
    Plugin *mPlugin = nullptr;
};
//@endcond

UniqueAppHandler::UniqueAppHandler(Plugin *plugin)
    : QObject(plugin)
    , d(new UniqueAppHandlerPrivate)
{
    qCDebug(KONTACTINTERFACE_LOG) << "plugin->objectName():" << plugin->objectName();

    d->mPlugin = plugin;
    QDBusConnection session = QDBusConnection::sessionBus();
    const QString appName = plugin->objectName();
    session.registerService(QLatin1String("org.kde.") + appName);
    const QString objectName = QLatin1Char('/') + appName + QLatin1String("_PimApplication");
    session.registerObject(objectName, this, QDBusConnection::ExportAllSlots);
}

UniqueAppHandler::~UniqueAppHandler()
{
    QDBusConnection session = QDBusConnection::sessionBus();
    const QString appName = parent()->objectName();
    session.unregisterService(QLatin1String("org.kde.") + appName);
}

// DBUS call
int UniqueAppHandler::newInstance(const QByteArray &asn_id, const QStringList &args, const QString &workingDirectory)
{
    if (!asn_id.isEmpty()) {
        KStartupInfo::setStartupId(asn_id);
    }

    QCommandLineParser parser;
    loadCommandLineOptions(&parser); // implemented by plugin
    parser.process(args);

    return activate(args, workingDirectory);
}

static QWidget *s_mainWidget = nullptr;

// Plugin-specific newInstance implementation, called by above method
int KontactInterface::UniqueAppHandler::activate(const QStringList &args, const QString &workingDirectory)
{
    Q_UNUSED(args)
    Q_UNUSED(workingDirectory)

    if (s_mainWidget) {
        s_mainWidget->show();
#ifdef Q_OS_WIN
        KWindowSystem::forceActiveWindow(s_mainWidget->winId());
#endif
        KStartupInfo::appStarted();
    }

    // Then ensure the part appears in kontact
    d->mPlugin->core()->selectPlugin(d->mPlugin);
    return 0;
}

Plugin *UniqueAppHandler::plugin() const
{
    return d->mPlugin;
}

bool KontactInterface::UniqueAppHandler::load()
{
    (void)d->mPlugin->part(); // load the part without bringing it to front
    return true;
}

//@cond PRIVATE
class Q_DECL_HIDDEN UniqueAppWatcher::UniqueAppWatcherPrivate
{
public:
    UniqueAppHandlerFactoryBase *mFactory = nullptr;
    Plugin *mPlugin = nullptr;
    bool mRunningStandalone;
};
//@endcond

UniqueAppWatcher::UniqueAppWatcher(UniqueAppHandlerFactoryBase *factory, Plugin *plugin)
    : QObject(plugin)
    , d(new UniqueAppWatcherPrivate)
{
    d->mFactory = factory;
    d->mPlugin = plugin;

    // The app is running standalone if 1) that name is known to D-Bus
    const QString serviceName = QLatin1String("org.kde.") + plugin->objectName();
    // Needed for wince build
#undef interface
    d->mRunningStandalone = QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName);
#ifdef Q_OS_WIN
    if (d->mRunningStandalone) {
        QList<int> pids;
        getProcessesIdForName(plugin->objectName(), pids);
        const int mypid = getpid();
        bool processExits = false;
        for (int pid : std::as_const(pids)) {
            if (mypid != pid) {
                processExits = true;
                break;
            }
        }
        if (!processExits) {
            d->mRunningStandalone = false;
        }
    }
#endif

    QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(serviceName);
    if (d->mRunningStandalone && (owner == QDBusConnection::sessionBus().baseService())) {
        d->mRunningStandalone = false;
    }

    qCDebug(KONTACTINTERFACE_LOG) << " plugin->objectName()=" << plugin->objectName() << " running standalone:" << d->mRunningStandalone;

    if (d->mRunningStandalone) {
        QObject::connect(QDBusConnection::sessionBus().interface(),
                         &QDBusConnectionInterface::serviceOwnerChanged,
                         this,
                         &UniqueAppWatcher::slotApplicationRemoved);
    } else {
        d->mFactory->createHandler(d->mPlugin);
    }
}

UniqueAppWatcher::~UniqueAppWatcher()
{
    delete d->mFactory;
}

bool UniqueAppWatcher::isRunningStandalone() const
{
    return d->mRunningStandalone;
}

void KontactInterface::UniqueAppWatcher::slotApplicationRemoved(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (oldOwner.isEmpty() || !newOwner.isEmpty()) {
        return;
    }

    const QString serviceName = QLatin1String("org.kde.") + d->mPlugin->objectName();
    if (name == serviceName && d->mRunningStandalone) {
        d->mFactory->createHandler(d->mPlugin);
        d->mRunningStandalone = false;
    }
}

void KontactInterface::UniqueAppHandler::setMainWidget(QWidget *widget)
{
    s_mainWidget = widget;
}

QWidget *KontactInterface::UniqueAppHandler::mainWidget()
{
    return s_mainWidget;
}
