/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "plugin.h"
#include <QFile>
#include "core.h"

#include "processes.h"
#include <kparts/factory.h>
#include <kpluginloader.h>

#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocalizedstring.h>
#include <qdebug.h>
#include <kcomponentdata.h>

#include <krun.h>
#include <kaboutdata.h>

#include <QObject>
#include <QDBusConnection>
#include <QDomDocument>
#include <QFileInfo>
#include <QDir>

#include <unistd.h>
#include <QStandardPaths>

using namespace KontactInterface;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Plugin::Private
{
public:

    void partDestroyed();
    void setXmlFiles();
    void removeInvisibleToolbarActions(Plugin *plugin);

    Core *core;
    QList<QAction *> newActions;
    QList<QAction *> syncActions;
    QString identifier;
    QString title;
    QString icon;
    QString executableName;
    QString serviceName;
    QByteArray partLibraryName;
    QByteArray pluginName;
    bool hasPart;
    KParts::ReadOnlyPart *part;
    bool disabled;
};
//@endcond

Plugin::Plugin(Core *core, QObject *parent, const char *appName, const char *pluginName)
    : KXMLGUIClient(core), QObject(parent), d(new Private)
{
    setObjectName(QLatin1String(appName));
    core->factory()->addClient(this);

    d->pluginName = pluginName ? pluginName : appName;
    d->core = core;
    d->hasPart = true;
    d->part = 0;
    d->disabled = false;
}

Plugin::~Plugin()
{
    delete d->part;
    delete d;
}

void Plugin::setIdentifier(const QString &identifier)
{
    d->identifier = identifier;
}

QString Plugin::identifier() const
{
    return d->identifier;
}

void Plugin::setTitle(const QString &title)
{
    d->title = title;
}

QString Plugin::title() const
{
    return d->title;
}

void Plugin::setIcon(const QString &icon)
{
    d->icon = icon;
}

QString Plugin::icon() const
{
    return d->icon;
}

void Plugin::setExecutableName(const QString &bin)
{
    d->executableName = bin;
}

QString Plugin::executableName() const
{
    return d->executableName;
}

void Plugin::setPartLibraryName(const QByteArray &libName)
{
    d->partLibraryName = libName;
}

bool Plugin::createDBUSInterface(const QString &serviceType)
{
    Q_UNUSED(serviceType);
    return false;
}

bool Plugin::isRunningStandalone() const
{
    return false;
}

KParts::ReadOnlyPart *Plugin::loadPart()
{
    return core()->createPart(d->partLibraryName.constData());
}

const KAboutData Plugin::aboutData()
{
    return part()->componentData();
}

KParts::ReadOnlyPart *Plugin::part()
{
    if (!d->part) {
        d->part = createPart();
        if (d->part) {
            connect(d->part, SIGNAL(destroyed()), SLOT(partDestroyed()));
            d->removeInvisibleToolbarActions(this);
            core()->partLoaded(this, d->part);
        }
    }
    return d->part;
}

QString Plugin::tipFile() const
{
    return QString();
}

QString Plugin::registerClient()
{
    if (d->serviceName.isEmpty()) {
        d->serviceName = QLatin1String("org.kde.") + QLatin1String(objectName().toLatin1());
#ifdef Q_OS_WIN
        const QString pid = QString::number(getpid());
        d->serviceName.append(QLatin1String(".unique-") + pid);
#endif
        QDBusConnection::sessionBus().registerService(d->serviceName);
    }
    return d->serviceName;
}

int Plugin::weight() const
{
    return 0;
}

void Plugin::insertNewAction(QAction *action)
{
    d->newActions.append(action);
}

void Plugin::insertSyncAction(QAction *action)
{
    d->syncActions.append(action);
}

QList<QAction *> Plugin::newActions() const
{
    return d->newActions;
}

QList<QAction *> Plugin::syncActions() const
{
    return d->syncActions;
}

QStringList Plugin::invisibleToolbarActions() const
{
    return QStringList();
}

bool Plugin::canDecodeMimeData(const QMimeData *data) const
{
    Q_UNUSED(data);
    return false;
}

void Plugin::processDropEvent(QDropEvent *)
{
}

void Plugin::readProperties(const KConfigGroup &)
{
}

void Plugin::saveProperties(KConfigGroup &)
{
}

Core *Plugin::core() const
{
    return d->core;
}

void Plugin::aboutToSelect()
{
    // Because the 3 korganizer plugins share the same part, we need to switch
    // that part's XML files every time we are about to show its GUI...
    d->setXmlFiles();

    select();
}

void Plugin::select()
{
}

void Plugin::configUpdated()
{
}

//@cond PRIVATE
void Plugin::Private::partDestroyed()
{
    part = 0;
}

void Plugin::Private::removeInvisibleToolbarActions(Plugin *plugin)
{
    if (pluginName.isEmpty()) {
        return;
    }

    // Hide unwanted toolbar action by modifying the XML before createGUI, rather
    // than doing it by calling removeAction on the toolbar after createGUI. Both
    // solutions work visually, but only modifying the XML ensures that the
    // actions don't appear in "edit toolbars". #207296
    const QStringList hideActions = plugin->invisibleToolbarActions();
    //qDebug() << "Hiding actions" << hideActions << "from" << pluginName << part;
    QDomDocument doc = part->domDocument();
    QDomElement docElem = doc.documentElement();
    // 1. Iterate over containers
    for (QDomElement containerElem = docElem.firstChildElement();
            !containerElem.isNull(); containerElem = containerElem.nextSiblingElement()) {
        if (QString::compare(containerElem.tagName(), QLatin1String("ToolBar"), Qt::CaseInsensitive) == 0) {
            // 2. Iterate over actions in toolbars
            QDomElement actionElem = containerElem.firstChildElement();
            while (!actionElem.isNull()) {
                QDomElement nextActionElem = actionElem.nextSiblingElement();
                if (QString::compare(actionElem.tagName(), QLatin1String("Action"), Qt::CaseInsensitive) == 0) {
                    //qDebug() << "Looking at action" << actionElem.attribute("name");
                    if (hideActions.contains(actionElem.attribute(QLatin1String("name")))) {
                        //qDebug() << "REMOVING";
                        containerElem.removeChild(actionElem);
                    }
                }
                actionElem = nextActionElem;
            }
        }
    }

    // Possible optimization: we could do all the above and the writing below
    // only when (newAppFile does not exist) or (version of domDocument > version of newAppFile)  (*)
    // This requires parsing newAppFile when it exists, though, and better use
    // the fast kdeui code for that rather than a full QDomDocument.
    // (*) or when invisibleToolbarActions() changes :)

    const QString newAppFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kontact/default-") + QLatin1String(pluginName) + QLatin1String(".rc") ;
    QFileInfo fileInfo(newAppFile);
    QDir().mkpath(fileInfo.absolutePath());

    QFile file(newAppFile);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "error writing to" << newAppFile;
        return;
    }
    file.write(doc.toString().toUtf8());
    file.flush();

    setXmlFiles();
}

void Plugin::Private::setXmlFiles()
{
    const QString newAppFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kontact/default-") + QLatin1String(pluginName) + QLatin1String(".rc") ;
    const QString localFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kontact/local-") + QLatin1String(pluginName) + QLatin1String(".rc") ;
    if (part->xmlFile() != newAppFile || part->localXMLFile() != localFile) {
        part->replaceXMLFile(newAppFile, localFile);
    }
}
//@endcond

void Plugin::slotConfigUpdated()
{
    configUpdated();
}

void Plugin::bringToForeground()
{
    if (d->executableName.isEmpty()) {
        return;
    }
#ifdef Q_OS_WIN
    activateWindowForProcess(d->executableName);
#else
    KRun::runCommand(d->executableName, 0);
#endif
}

Summary *Plugin::createSummaryWidget(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

bool Plugin::showInSideBar() const
{
    return d->hasPart;
}

void Plugin::setShowInSideBar(bool hasPart)
{
    d->hasPart = hasPart;
}

bool Plugin::queryClose() const
{
    return true;
}

void Plugin::setDisabled(bool disabled)
{
    d->disabled = disabled;
}

bool Plugin::disabled() const
{
    return d->disabled;
}

void Plugin::shortcutChanged()
{
}

void Plugin::virtual_hook(int, void *)
{
    //BASE::virtual_hook( id, data );
}

#include "moc_plugin.cpp"

