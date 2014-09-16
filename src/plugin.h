/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KONTACTINTERFACE_PLUGIN_H
#define KONTACTINTERFACE_PLUGIN_H

#include "kontactinterface_export.h"

#include <kpluginfactory.h>
#include <kxmlguiclient.h>

#include <QtCore/QList>
#include <QtCore/QObject>

class KAboutData;
class QAction;
class KConfig;
class KConfigGroup;
class QDropEvent;
class QMimeData;
class QStringList;
class QWidget;
namespace KParts
{
class ReadOnlyPart;
}

/**
  Exports Kontact plugin.
 */
#define EXPORT_KONTACT_PLUGIN( pluginclass, pluginname ) \
    class Instance                                           \
    {                                                        \
    public:                                                \
        static QObject *createInstance( QWidget *, QObject *parent, const QVariantList &list ) \
        { return new pluginclass( static_cast<KontactInterface::Core*>( parent ), list ); } \
    };                                                                    \
    K_PLUGIN_FACTORY( KontactPluginFactory, registerPlugin< pluginclass >   \
                      ( QString(), Instance::createInstance ); )            \
    K_EXPORT_PLUGIN( KontactPluginFactory( "kontact_" #pluginname "plugin" ) )

/**
  Increase this version number whenever you make a change in the API.
 */
#define KONTACT_PLUGIN_VERSION 10

namespace KontactInterface
{

class Core;
class Summary;
/**
 * @short Base class for all Plugins in Kontact.
 *
 * Inherit from it to get a plugin. It can insert an icon into the sidepane,
 * add widgets to the widgetstack and add menu items via XMLGUI.
 */
class KONTACTINTERFACE_EXPORT Plugin : public QObject, virtual public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Creates a new plugin.
     *
     * @param core The core object that manages the plugin.
     * @param parent The parent object.
     * @param appName The name of the application that
     *       provides the part. This is the name used for DBus registration.
     *       It's ok to have several plugins using the same application name.
     * @param pluginName The unique name of the plugin. Defaults to appName if not set.
     */
    Plugin(Core *core, QObject *parent, const char *appName, const char *pluginName = 0);

    /**
     * Destroys the plugin.
     */
    virtual ~Plugin();

    /**
     * Sets the @p identifier of the plugin.
     */
    void setIdentifier(const QString &identifier);

    /**
     * Returns the identifier of the plugin.
     */
    QString identifier() const;

    /**
     * Sets the localized @p title of the plugin.
     */
    void setTitle(const QString &title);

    /**
     * Returns the localized title of the plugin.
     */
    QString title() const;

    /**
     * Sets the @p icon name that is used for the plugin.
     */
    void setIcon(const QString &icon);

    /**
     * Returns the icon name that is used for the plugin.
     */
    QString icon() const;

    /**
     * Sets the @p name of executable (if existent).
     */
    void setExecutableName(const QString &name);

    /**
     * Returns the name of the executable (if existent).
     */
    QString executableName() const;

    /**
     * Set @p name of library which contains the KPart used by this plugin.
     */
    void setPartLibraryName(const QByteArray &name);

    /**
     * Create the D-Bus interface for the given @p serviceType, if this
     * plugin provides it. Returns @c true on success, @c false otherwise.
     * @param serviceType the D-Bus service type to create an interface for
     */
    virtual bool createDBUSInterface(const QString &serviceType);

    /**
     * Reimplement this method and return whether a standalone application
     * is still running. This is only required if your part is also available
     * as standalone application.
     */
    virtual bool isRunningStandalone() const;

    /**
     * Reimplement this method if your application needs a different approach to be brought
     * in the foreground. The default behaviour is calling the binary.
     * This is only required if your part is also available as standalone application.
     */
    virtual void bringToForeground();

    /**
     * Reimplement this method if you want to add your credits to the Kontact
     * about dialog.
     */
    virtual const KAboutData aboutData();

    /**
     * You can use this method if you need to access the current part. You can be
     * sure that you always get the same pointer as long as the part has not been
     * deleted.
     */
    KParts::ReadOnlyPart *part();

    /**
     * Reimplement this method and return the a path relative to "data" to the tips file.
     * The tips file contains hints/tips that are displayed at the beginning of the program
     * as "tip of the day". It has nothing to do with tooltips.
     */
    virtual QString tipFile() const;

    /**
     * This function is called when the plugin is selected by the user before the
     * widget of the KPart belonging to the plugin is raised.
     */
    virtual void select();

    /**
     * Called by kontact when the plugin is selected by the user.
     * Calls the virtual method select(), but also handles some standard behavior
     * like "invisible toolbar actions".
     */
    void aboutToSelect();

    /**
     * This function is called whenever the config dialog has been closed
     * successfully.
     */
    virtual void configUpdated();

    /**
     * Reimplement this method if you want to add a widget for your application
     * to Kontact's summary page.
     *
     * @param parent The parent widget of the summary widget.
     */
    virtual Summary *createSummaryWidget(QWidget *parent);

    /**
     * Returns whether the plugin provides a part that should be shown in the sidebar.
     */
    virtual bool showInSideBar() const;

    /**
     * Set if the plugin provides a part that should be shown in the sidebar.
     * @param hasPart shows part in sidebar if set as @c true
     */
    void setShowInSideBar(bool hasPart);

    /**
     * Reimplement this method if you want to add checks before closing the
     * main kontact window. Return true if it's OK to close the window.
     * If any loaded plugin returns false from this method, then the
     * main kontact window will not close.
     */
    virtual bool queryClose() const;

    /**
     * Registers the client at DBus and returns the dbus identifier.
     */
    QString registerClient();

    /**
     * Return the weight of the plugin. The higher the weight the lower it will
     * be displayed in the sidebar. The default implementation returns 0.
     */
    virtual int weight() const;

    /**
     * Inserts a custom "New" @p action.
     * @param action the new action to insert
     */
    void insertNewAction(QAction *action);

    /**
     * Inserts a custom "Sync" @p action.
     * @param action the custom Sync action to insert
     */
    void insertSyncAction(QAction *action);

    /**
     * Returns the list of custom "New" actions.
     */
    QList<QAction *> newActions() const;

    /**
     * Returns the list of custom "Sync" actions.
     */
    QList<QAction *> syncActions() const;

    /**
     * Returns a list of action names that shall be hidden in the main toolbar.
     */
    virtual QStringList invisibleToolbarActions() const;

    /**
     * Returns whether the plugin can handle the drag object of the given mime type.
     */
    virtual bool canDecodeMimeData(const QMimeData *data) const;

    /**
     * Process drop event.
     */
    virtual void processDropEvent(QDropEvent *);

    /**
     * Session management: read properties
     */
    virtual void readProperties(const KConfigGroup &);

    /**
     * Session management: save properties
     */
    virtual void saveProperties(KConfigGroup &);

    /**
     * Returns a pointer to the kontact core object.
     */
    Core *core() const;

    /**
     * Sets whether the plugin shall be disabled.
     */
    void setDisabled(bool value);

    /**
     * Returns whether the plugin is disabled.
     */
    bool disabled() const;

    /**
     * @since 4.13
     */
    virtual void shortcutChanged();

public Q_SLOTS:
    /**
     * @internal usage
     *
     * This slot is called whenever the configuration has been changed.
     */
    void slotConfigUpdated();

protected:
    /**
     * Reimplement and return the part here. Reimplementing createPart() is
     * mandatory!
     */
    virtual KParts::ReadOnlyPart *createPart() = 0;

    /**
     * Returns the loaded part.
     */
    KParts::ReadOnlyPart *loadPart();

    /**
     * Virtual hook for BC extension.
     */
    virtual void virtual_hook(int id, void *data);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void partDestroyed())
    //@endcond
};

}

#endif
