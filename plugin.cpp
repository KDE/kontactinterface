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

#include <kpimutils/processes.h>

#include <kparts/componentfactory.h>
#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocalizedstring.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <krun.h>

#include <QObject>
#include <QDBusConnection>
#include <QDomDocument>

#include <unistd.h>

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
    void removeInvisibleToolbarActions( Plugin *plugin );

    Core *core;
    QList<KAction*> newActions;
    QList<KAction*> syncActions;
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

Plugin::Plugin( Core *core, QObject *parent, const char *appName, const char *pluginName )
  : KXMLGUIClient( core ), QObject( parent ), d( new Private )
{
  setObjectName( appName );
  core->factory()->addClient( this );
  KLocalizedString::insertCatalog( appName );

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

void Plugin::setIdentifier( const QString &identifier )
{
  d->identifier = identifier;
}

QString Plugin::identifier() const
{
  return d->identifier;
}

void Plugin::setTitle( const QString &title )
{
  d->title = title;
}

QString Plugin::title() const
{
  return d->title;
}

void Plugin::setIcon( const QString &icon )
{
  d->icon = icon;
}

QString Plugin::icon() const
{
  return d->icon;
}

void Plugin::setExecutableName( const QString &bin )
{
  d->executableName = bin;
}

QString Plugin::executableName() const
{
  return d->executableName;
}

void Plugin::setPartLibraryName( const QByteArray &libName )
{
  d->partLibraryName = libName;
}

bool Plugin::createDBUSInterface( const QString &serviceType )
{
  Q_UNUSED( serviceType );
  return false;
}

bool Plugin::isRunningStandalone() const
{
  return false;
}

KParts::ReadOnlyPart *Plugin::loadPart()
{
  return core()->createPart( d->partLibraryName );
}

const KAboutData *Plugin::aboutData() const
{
  KPluginLoader loader( d->partLibraryName );
  KPluginFactory *factory = loader.factory();
  kDebug() << "filename:" << loader.fileName();
  kDebug() << "libname:" << d->partLibraryName;

  if ( factory ) {
    if ( factory->componentData().isValid() ) {
      kDebug() << "returning factory component aboutdata";
      return factory->componentData().aboutData();
    } else {
      // If the componentData of the factory is invalid, the likely cause is that
      // the part has not been ported to use K_PLUGIN_FACTORY/K_EXPORT_PLUGIN yet.
      // In that case, fallback to the old method of loading component data, which
      // does only work for old-style parts.

      kDebug() << "Unable to load component data for" << loader.fileName()
               << "trying to use the old style plugin system now.";
      const KComponentData instance =
        KParts::Factory::partComponentDataFromLibrary( d->partLibraryName );
      if ( instance.isValid() ) {
        return instance.aboutData();
      } else {
        kDebug() << "Invalid instance, unable to get about information!";
      }
    }
  }

  kError() << "Cannot load instance for" << title();
  return 0;
}

KParts::ReadOnlyPart *Plugin::part()
{
  if ( !d->part ) {
    d->part = createPart();
    if ( d->part ) {
      connect( d->part, SIGNAL(destroyed()), SLOT(partDestroyed()) );
      d->removeInvisibleToolbarActions( this );
      core()->partLoaded( this, d->part );
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
  if ( d->serviceName.isEmpty() ) {
    d->serviceName = "org.kde." + objectName().toLatin1();
#ifdef Q_WS_WIN
    const QString pid = QString::number( getpid() );
    d->serviceName.append( ".unique-" + pid );
#endif
    QDBusConnection::sessionBus().registerService( d->serviceName );
  }
  return d->serviceName;
}

int Plugin::weight() const
{
  return 0;
}

void Plugin::insertNewAction( KAction *action )
{
  d->newActions.append( action );
}

void Plugin::insertSyncAction( KAction *action )
{
  d->syncActions.append( action );
}

QList<KAction*> Plugin::newActions() const
{
  return d->newActions;
}

QList<KAction*> Plugin::syncActions() const
{
  return d->syncActions;
}

QStringList Plugin::invisibleToolbarActions() const
{
  return QStringList();
}

bool Plugin::canDecodeMimeData( const QMimeData *data ) const
{
  Q_UNUSED( data );
  return false;
}

void Plugin::processDropEvent( QDropEvent * )
{
}

void Plugin::readProperties( const KConfigGroup & )
{
}

void Plugin::saveProperties( KConfigGroup & )
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

void Plugin::Private::removeInvisibleToolbarActions( Plugin *plugin )
{
  if ( pluginName.isEmpty() ) {
    return;
  }

  // Hide unwanted toolbar action by modifying the XML before createGUI, rather
  // than doing it by calling removeAction on the toolbar after createGUI. Both
  // solutions work visually, but only modifying the XML ensures that the
  // actions don't appear in "edit toolbars". #207296
  const QStringList hideActions = plugin->invisibleToolbarActions();
  //kDebug() << "Hiding actions" << hideActions << "from" << pluginName << part;
  QDomDocument doc = part->domDocument();
  QDomElement docElem = doc.documentElement();
  // 1. Iterate over containers
  for ( QDomElement containerElem = docElem.firstChildElement();
        !containerElem.isNull(); containerElem = containerElem.nextSiblingElement() ) {
    if ( QString::compare( containerElem.tagName(), "ToolBar", Qt::CaseInsensitive ) == 0 ) {
      // 2. Iterate over actions in toolbars
      QDomElement actionElem = containerElem.firstChildElement();
      while ( !actionElem.isNull() ) {
        QDomElement nextActionElem = actionElem.nextSiblingElement();
        if ( QString::compare( actionElem.tagName(), "Action", Qt::CaseInsensitive ) == 0 ) {
          //kDebug() << "Looking at action" << actionElem.attribute("name");
          if ( hideActions.contains( actionElem.attribute( "name" ) ) ) {
            //kDebug() << "REMOVING";
            containerElem.removeChild( actionElem );
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
    KStandardDirs::locateLocal( "data", "kontact/default-" + pluginName + ".rc" );
  QFile file( newAppFile );
  if ( !file.open( QFile::WriteOnly ) ) {
    kWarning() << "error writing to" << newAppFile;
    return;
  }
  file.write( doc.toString().toUtf8() );
  file.flush();

  setXmlFiles();
}

void Plugin::Private::setXmlFiles()
{
  const QString newAppFile =
    KStandardDirs::locateLocal( "data", "kontact/default-" + pluginName + ".rc" );
  const QString localFile =
    KStandardDirs::locateLocal( "data", "kontact/local-" + pluginName + ".rc" );
  if ( part->xmlFile() != newAppFile || part->localXMLFile() != localFile ) {
    part->replaceXMLFile( newAppFile, localFile );
  }
}
//@endcond

void Plugin::slotConfigUpdated()
{
  configUpdated();
}

void Plugin::bringToForeground()
{
  if ( d->executableName.isEmpty() ) {
    return;
  }
#ifdef Q_WS_WIN
  KPIMUtils::activateWindowForProcess( d->executableName );
#else
  KRun::runCommand( d->executableName, 0 );
#endif
}

Summary *Plugin::createSummaryWidget( QWidget *parent )
{
  Q_UNUSED( parent );
  return 0;
}

bool Plugin::showInSideBar() const
{
  return d->hasPart;
}

void Plugin::setShowInSideBar( bool hasPart )
{
  d->hasPart = hasPart;
}

bool Plugin::queryClose() const
{
  return true;
}

void Plugin::setDisabled( bool disabled )
{
  d->disabled = disabled;
}

bool Plugin::disabled() const
{
  return d->disabled;
}

void Plugin::virtual_hook( int, void * )
{
  //BASE::virtual_hook( id, data );
}

#include "moc_plugin.cpp"

// vim: sw=2 et sts=2 tw=80
