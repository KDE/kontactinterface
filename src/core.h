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
#ifndef KONTACTINTERFACE_CORE_H
#define KONTACTINTERFACE_CORE_H

#include "kontactinterface_export.h"
#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <KParts/ReadOnlyPart>
namespace KontactInterface
{

class Plugin;

/**
 * @short The abstract interface that represents the Kontact core.
 *
 * This class provides the interface to the Kontact core for the plugins.
 */
class KONTACTINTERFACE_EXPORT Core : public KParts::MainWindow
{
    Q_OBJECT

public:
    /**
     * Destroys the core object.
     */
    virtual ~Core();

    /**
     * Selects the given plugin and raises the associated part.
     * @see selectPlugin(const QString &)
     *
     * @param plugin is a pointer to the Kontact Plugin to select.
     */
    virtual void selectPlugin(KontactInterface::Plugin *plugin) = 0;

    /**
     * This is an overloaded member function
     * @see selectPlugin(KontactInterface::Plugin *)
     *
     * @param plugin is the name of the Kontact Plugin select.
     */
    virtual void selectPlugin(const QString &plugin) = 0;

    /**
     * Returns the pointer list of available plugins.
     */
    virtual QList<KontactInterface::Plugin *> pluginList() const = 0;

    /**
     * @internal (for Plugin)
     *
     * @param library the library to create part from
     * Creates a part from the given @p library.
     */
    KParts::ReadOnlyPart *createPart(const char *library);

    /**
     * @internal (for Plugin)
     *
     * Tells the kontact core that a part has been loaded.
     */
    virtual void partLoaded(Plugin *plugin, KParts::ReadOnlyPart *part) = 0;

Q_SIGNALS:
    /**
     * This signal is emitted whenever a new day starts.
     *
     * @param date The date of the new day
     */
    void dayChanged(const QDate &date);

protected:
    /**
     * Creates a new core object.
     *
     * @param parent The parent widget.
     * @param flags The window flags.
     */
    explicit Core(QWidget *parent = 0, Qt::WindowFlags flags = KDE_DEFAULT_WINDOWFLAGS);

    /**
     * Returns the last error message for problems during
     * KParts loading.
     */
    QString lastErrorMessage() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void slotPartDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void checkNewDay())
    //@endcond
};

}

#endif

