/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/
#ifndef KONTACTINTERFACE_CORE_H
#define KONTACTINTERFACE_CORE_H

#include "kontactinterface_export.h"

#include <KParts/MainWindow>
#include <KParts/Part>

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
    ~Core() override;

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
    Q_REQUIRED_RESULT KParts::Part *createPart(const char *library);

    /**
     * @internal (for Plugin)
     *
     * Tells the kontact core that a part has been loaded.
     */
    virtual void partLoaded(Plugin *plugin, KParts::Part *part) = 0;

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
    explicit Core(QWidget *parent = nullptr, Qt::WindowFlags flags = {});

    /**
     * Returns the last error message for problems during
     * KParts loading.
     */
    QString lastErrorMessage() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
