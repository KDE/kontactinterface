/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugin.h"
using namespace Qt::Literals::StringLiterals;

#include "core.h"
#include "kontactinterface_debug.h"
#include "processes.h"

#include <KAboutData>
#include <KIO/CommandLauncherJob>
#include <KXMLGUIFactory>

#include <QDBusConnection>
#include <QDir>
#include <QDomDocument>
#include <QFileInfo>

#include <QCoreApplication>
#include <QStandardPaths>

using namespace KontactInterface;

/**
  PluginPrivate class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN Plugin::PluginPrivate
{
public:
    void partDestroyed();
    void setXmlFiles();
    void removeInvisibleToolbarActions(Plugin *plugin);

    Core *core = nullptr;
    QList<QAction *> newActions;
    QList<QAction *> syncActions;
    QString identifier;
    QString title;
    QString icon;
    QString executableName;
    QString serviceName;
    QByteArray partLibraryName;
    QByteArray pluginName;
    KParts::Part *part = nullptr;
    bool hasPart = true;
    bool disabled = false;
};
//@endcond
Plugin::Plugin(Core *core, QObject *parent, const KPluginMetaData &, const char *appName, const char *pluginName)
    : KXMLGUIClient(core)
    , QObject(parent)
    , d(new PluginPrivate)
{
    setObjectName(QLatin1StringView(appName));
    core->factory()->addClient(this);

    d->pluginName = pluginName ? pluginName : appName;
    d->core = core;
}

Plugin::~Plugin()
{
    delete d->part;
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

bool Plugin::isRunningStandalone() const
{
    return false;
}

KParts::Part *Plugin::loadPart()
{
    return core()->createPart(d->partLibraryName.constData());
}

const KAboutData Plugin::aboutData()
{
    return KAboutData();
}

KParts::Part *Plugin::part()
{
    if (!d->part) {
        d->part = createPart();
        if (d->part) {
            connect(d->part, &KParts::Part::destroyed, this, [this]() {
                d->partDestroyed();
            });
            d->removeInvisibleToolbarActions(this);
            core()->partLoaded(this, d->part);
        }
    }
    return d->part;
}

QString Plugin::registerClient()
{
    if (d->serviceName.isEmpty()) {
        d->serviceName = "org.kde."_L1 + QLatin1StringView(objectName().toLatin1());
#ifdef Q_OS_WIN
        const QString pid = QString::number(QCoreApplication::applicationPid());
        d->serviceName.append(".unique-"_L1 + pid);
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
    return {};
}

bool Plugin::canDecodeMimeData(const QMimeData *data) const
{
    Q_UNUSED(data)
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
void Plugin::PluginPrivate::partDestroyed()
{
    part = nullptr;
}

void Plugin::PluginPrivate::removeInvisibleToolbarActions(Plugin *plugin)
{
    if (pluginName.isEmpty()) {
        return;
    }

    // Hide unwanted toolbar action by modifying the XML before createGUI, rather
    // than doing it by calling removeAction on the toolbar after createGUI. Both
    // solutions work visually, but only modifying the XML ensures that the
    // actions don't appear in "edit toolbars". #207296
    const QStringList hideActions = plugin->invisibleToolbarActions();
    // qCDebug(KONTACTINTERFACE_LOG) << "Hiding actions" << hideActions << "from" << pluginName << part;
    const QDomDocument doc = part->domDocument();
    const QDomElement docElem = doc.documentElement();
    // 1. Iterate over containers
    for (QDomElement containerElem = docElem.firstChildElement(); !containerElem.isNull(); containerElem = containerElem.nextSiblingElement()) {
        if (QString::compare(containerElem.tagName(), "ToolBar"_L1, Qt::CaseInsensitive) == 0) {
            // 2. Iterate over actions in toolbars
            QDomElement actionElem = containerElem.firstChildElement();
            while (!actionElem.isNull()) {
                QDomElement nextActionElem = actionElem.nextSiblingElement();
                if (QString::compare(actionElem.tagName(), "Action"_L1, Qt::CaseInsensitive) == 0) {
                    // qCDebug(KONTACTINTERFACE_LOG) << "Looking at action" << actionElem.attribute("name");
                    if (hideActions.contains(actionElem.attribute(QStringLiteral("name")))) {
                        // qCDebug(KONTACTINTERFACE_LOG) << "REMOVING";
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
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kontact/default-"_L1 + QLatin1StringView(pluginName) + ".rc"_L1;
    const QFileInfo fileInfo(newAppFile);
    QDir().mkpath(fileInfo.absolutePath());

    QFile file(newAppFile);
    if (!file.open(QFile::WriteOnly)) {
        qCWarning(KONTACTINTERFACE_LOG) << "error writing to" << newAppFile;
        return;
    }
    file.write(doc.toString().toUtf8());
    file.flush();

    setXmlFiles();
}

void Plugin::PluginPrivate::setXmlFiles()
{
    if (pluginName.isEmpty()) {
        return;
    }
    const QString newAppFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kontact/default-"_L1 + QLatin1StringView(pluginName) + ".rc"_L1;
    const QString localFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kontact/local-"_L1 + QLatin1StringView(pluginName) + ".rc"_L1;
    if (!localFile.isEmpty() && !newAppFile.isEmpty()) {
        if (part->xmlFile() != newAppFile || part->localXMLFile() != localFile) {
            part->replaceXMLFile(newAppFile, localFile);
        }
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
    auto job = new KIO::CommandLauncherJob(d->executableName);
    job->start();
#endif
}

Summary *Plugin::createSummaryWidget(QWidget *parent)
{
    Q_UNUSED(parent)
    return nullptr;
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
    // BASE::virtual_hook( id, data );
}

#include "moc_plugin.cpp"
