/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2003, 2008 David Faure <faure@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kontactinterface_export.h"
#include "plugin.h"

class QCommandLineParser;

namespace KontactInterface
{
/**
 * D-Bus Object that has the name of the standalone application (e.g. "kmail")
 * and implements newInstance() so that running the separate application does
 * the right thing when kontact is running.
 * By default this means simply bringing the main window to the front,
 * but newInstance can be reimplemented.
 */
class KONTACTINTERFACE_EXPORT UniqueAppHandler : public QObject
{
    Q_OBJECT
    // We implement the PIMUniqueApplication interface
    Q_CLASSINFO("D-Bus Interface", "org.kde.PIMUniqueApplication")

public:
    explicit UniqueAppHandler(Plugin *plugin);
    ~UniqueAppHandler() override;

    /// This must be reimplemented so that app-specific command line options can be parsed
    virtual void loadCommandLineOptions(QCommandLineParser *parser) = 0;

    Q_REQUIRED_RESULT Plugin *plugin() const;

    /**
      Sets the main QWidget @p widget associated with this application.
      @param widget the QWidget to set as main
    */
    static void setMainWidget(QWidget *widget);

    /**
      Returns the main widget, which will zero if setMainWidget() has not be called yet.
      @since 4.6
    */
    Q_REQUIRED_RESULT QWidget *mainWidget();

public Q_SLOTS: // DBUS methods
    int newInstance(const QByteArray &asn_id, const QStringList &args, const QString &workingDirectory);
    bool load();

protected:
    virtual int activate(const QStringList &args, const QString &workingDirectory);

private:
    class Private;
    Private *const d;
};

/// Base class for UniqueAppHandler
class UniqueAppHandlerFactoryBase
{
public:
    virtual ~UniqueAppHandlerFactoryBase()
    {
    }
    virtual UniqueAppHandler *createHandler(Plugin *) = 0;
};

/**
 * Used by UniqueAppWatcher below, to create the above UniqueAppHandler object
 * when necessary.
 * The template argument is the UniqueAppHandler-derived class.
 * This allows to remove the need to subclass UniqueAppWatcher.
 */
template<class T> class UniqueAppHandlerFactory : public UniqueAppHandlerFactoryBase
{
public:
    UniqueAppHandler *createHandler(Plugin *plugin) override
    {
        plugin->registerClient();
        return new T(plugin);
    }
};

/**
 * If the standalone application is running by itself, we need to watch
 * for when the user closes it, and activate the uniqueapphandler then.
 * This prevents, on purpose, that the standalone app can be restarted.
 * Kontact takes over from there.
 *
 */
class KONTACTINTERFACE_EXPORT UniqueAppWatcher : public QObject
{
    Q_OBJECT

public:
    /**
     * Create an instance of UniqueAppWatcher, which does everything necessary
     * for the "unique application" behavior: create the UniqueAppHandler as soon
     * as possible, i.e. either right now or when the standalone app is closed.
     *
     * @param factory templatized factory to create the handler. Example:
     * ...   Note that the watcher takes ownership of the factory.
     * @param plugin is the plugin application
     */
    UniqueAppWatcher(UniqueAppHandlerFactoryBase *factory, Plugin *plugin);

    ~UniqueAppWatcher() override;

    bool isRunningStandalone() const;

private Q_SLOTS:
    void slotApplicationRemoved(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    class Private;
    Private *const d;
};

} // namespace

