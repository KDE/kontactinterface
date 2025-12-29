/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kontactinterface_export.h"

#include <KPluginFactory>
#include <KXMLGUIClient>

#include <QList>
#include <QObject>
#include <QStringList>

class KAboutData;
class QAction;
class KConfig;
class KConfigGroup;
class QDropEvent;
class QMimeData;
class QWidget;
namespace KParts
{
class Part;
}

/*!
  Increase this version number whenever you make a change in the API.
 */
#define KONTACT_PLUGIN_VERSION 11

/*!
  Exports Kontact plugin.
  \a pluginclass the class to instantiate (must derive from KontactInterface::Plugin)
  \a jsonFile filename of the JSON file, generated from a .desktop file
 */
#define EXPORT_KONTACT_PLUGIN_WITH_JSON(pluginclass, jsonFile)                                                                                                 \
    class Instance                                                                                                                                             \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        static QObject *createInstance(QWidget *, QObject *parent, const KPluginMetaData &data, const QVariantList &list)                                      \
        {                                                                                                                                                      \
            return new pluginclass(static_cast<KontactInterface::Core *>(parent), data, list);                                                                 \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    K_PLUGIN_FACTORY_WITH_JSON(KontactPluginFactory, jsonFile, registerPlugin<pluginclass>(Instance::createInstance);)

namespace KontactInterface
{
class Core;
class Summary;
/*!
 * \class KontactInterface::Plugin
 * \inmodule KontactInterface
 * \inheaderfile KontactInterface/Plugin
 *
 * \brief Base class for all Plugins in Kontact.
 *
 * Inherit from it to get a plugin. It can insert an icon into the sidepane,
 * add widgets to the widgetstack and add menu items via XMLGUI.
 */
class KONTACTINTERFACE_EXPORT Plugin : public QObject, virtual public KXMLGUIClient
{
    Q_OBJECT

public:
    /*!
     * Creates a new plugin.
     *
     * \a core The core object that manages the plugin.
     * \a parent The parent object.
     * \a appName The name of the application that
     *       provides the part. This is the name used for DBus registration.
     *       It's ok to have several plugins using the same application name.
     * \a pluginName The unique name of the plugin. Defaults to appName if not set.
     */
    Plugin(Core *core, QObject *parent, const KPluginMetaData &data, const char *appName, const char *pluginName = nullptr);
    /*!
     * Destroys the plugin.
     */
    ~Plugin() override;

    /*!
     * Sets the \a identifier of the plugin.
     */
    void setIdentifier(const QString &identifier);

    /*!
     * Returns the identifier of the plugin.
     */
    [[nodiscard]] QString identifier() const;

    /*!
     * Sets the localized \a title of the plugin.
     */
    void setTitle(const QString &title);

    /*!
     * Returns the localized title of the plugin.
     */
    [[nodiscard]] QString title() const;

    /*!
     * Sets the \a icon name that is used for the plugin.
     */
    void setIcon(const QString &icon);

    /*!
     * Returns the icon name that is used for the plugin.
     */
    [[nodiscard]] QString icon() const;

    /*!
     * Sets the \a name of executable (if existent).
     */
    void setExecutableName(const QString &name);

    /*!
     * Returns the name of the executable (if existent).
     */
    [[nodiscard]] QString executableName() const;

    /*!
     * Set \a name of library which contains the KPart used by this plugin.
     */
    void setPartLibraryName(const QByteArray &name);

    /*!
     * Reimplement this method and return whether a standalone application
     * is still running. This is only required if your part is also available
     * as standalone application.
     */
    [[nodiscard]] virtual bool isRunningStandalone() const;

    /*!
     * Reimplement this method if your application needs a different approach to be brought
     * in the foreground. The default behaviour is calling the binary.
     * This is only required if your part is also available as standalone application.
     */
    virtual void bringToForeground();

    /*!
     * Reimplement this method if you want to add your credits to the Kontact
     * about dialog.
     */
    [[nodiscard]] virtual const KAboutData aboutData();

    /*!
     * You can use this method if you need to access the current part. You can be
     * sure that you always get the same pointer as long as the part has not been
     * deleted.
     */
    [[nodiscard]] KParts::Part *part();

    /*!
     * This function is called when the plugin is selected by the user before the
     * widget of the KPart belonging to the plugin is raised.
     */
    virtual void select();

    /*!
     * Called by kontact when the plugin is selected by the user.
     * Calls the virtual method select(), but also handles some standard behavior
     * like "invisible toolbar actions".
     */
    void aboutToSelect();

    /*!
     * This function is called whenever the config dialog has been closed
     * successfully.
     */
    virtual void configUpdated();

    /*!
     * Reimplement this method if you want to add a widget for your application
     * to Kontact's summary page.
     *
     * \a parent The parent widget of the summary widget.
     */
    [[nodiscard]] virtual Summary *createSummaryWidget(QWidget *parent);

    /*!
     * Returns whether the plugin provides a part that should be shown in the sidebar.
     */
    [[nodiscard]] virtual bool showInSideBar() const;

    /*!
     * Set if the plugin provides a part that should be shown in the sidebar.
     * \a hasPart shows part in sidebar if set as true
     */
    void setShowInSideBar(bool hasPart);

    /*!
     * Reimplement this method if you want to add checks before closing the
     * main kontact window. Return true if it's OK to close the window.
     * If any loaded plugin returns false from this method, then the
     * main kontact window will not close.
     */
    [[nodiscard]] virtual bool queryClose() const;

    /*!
     * Registers the client at DBus and returns the dbus identifier.
     */
    QString registerClient();

    /*!
     * Return the weight of the plugin. The higher the weight the lower it will
     * be displayed in the sidebar. The default implementation returns 0.
     */
    virtual int weight() const;

    /*!
     * Inserts a custom "New" \a action.
     * \a action the new action to insert
     */
    void insertNewAction(QAction *action);

    /*!
     * Inserts a custom "Sync" \a action.
     * \a action the custom Sync action to insert
     */
    void insertSyncAction(QAction *action);

    /*!
     * Returns the list of custom "New" actions.
     */
    [[nodiscard]] QList<QAction *> newActions() const;

    /*!
     * Returns the list of custom "Sync" actions.
     */
    [[nodiscard]] QList<QAction *> syncActions() const;

    /*!
     * Returns a list of action names that shall be hidden in the main toolbar.
     */
    [[nodiscard]] virtual QStringList invisibleToolbarActions() const;

    /*!
     * Returns whether the plugin can handle the drag object of the given mime type.
     */
    [[nodiscard]] virtual bool canDecodeMimeData(const QMimeData *data) const;

    /*!
     * Process drop event.
     */
    virtual void processDropEvent(QDropEvent *);

    /*!
     * Session management: read properties
     */
    virtual void readProperties(const KConfigGroup &);

    /*!
     * Session management: save properties
     */
    virtual void saveProperties(KConfigGroup &);

    /*!
     * Returns a pointer to the kontact core object.
     */
    [[nodiscard]] Core *core() const;

    /*!
     * Sets whether the plugin shall be disabled.
     */
    void setDisabled(bool value);

    /*!
     * Returns whether the plugin is disabled.
     */
    [[nodiscard]] bool disabled() const;

    /*!
     * \since 4.13
     */
    virtual void shortcutChanged();

public Q_SLOTS:
    /*!
     * \internal usage
     *
     * This slot is called whenever the configuration has been changed.
     */
    void slotConfigUpdated();

protected:
    /*!
     * Reimplement and return the part here. Reimplementing createPart() is
     * mandatory!
     */
    virtual KParts::Part *createPart() = 0;

    /*!
     * Returns the loaded part.
     */
    KParts::Part *loadPart();

    /*!
     * Virtual hook for BC extension.
     */
    void virtual_hook(int id, void *data) override;

private:
    class PluginPrivate;
    std::unique_ptr<PluginPrivate> const d;
};

}
