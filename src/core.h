/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/
#pragma once

#include "kontactinterface_export.h"

#include <KParts/MainWindow>
#include <KParts/Part>

namespace KontactInterface
{
class Plugin;
class CorePrivate;
/*!
 * \class KontactInterface::Core
 * \inmodule KontactInterface
 * \inheaderfile KontactInterface/Core
 * \brief The abstract interface that represents the Kontact core.
 *
 * This class provides the interface to the Kontact core for the plugins.
 */
class KONTACTINTERFACE_EXPORT Core : public KParts::MainWindow
{
    Q_OBJECT

public:
    /*!
     * Destroys the core object.
     */
    ~Core() override;

    /*!
     * Selects the given plugin and raises the associated part.
     * \sa selectPlugin(const QString &)
     *
     * \a plugin is a pointer to the Kontact Plugin to select.
     */
    virtual void selectPlugin(KontactInterface::Plugin *plugin) = 0;

    /*!
     * This is an overloaded member function
     * \sa selectPlugin(KontactInterface::Plugin *)
     *
     * \a plugin is the name of the Kontact Plugin select.
     */
    virtual void selectPlugin(const QString &plugin) = 0;

    /*!
     * Returns the pointer list of available plugins.
     */
    virtual QList<KontactInterface::Plugin *> pluginList() const = 0;

    /*!
     * \internal (for Plugin)
     *
     * \a library the library to create part from
     * Creates a part from the given \a library.
     */
    [[nodiscard]] KParts::Part *createPart(const char *library);

    /*!
     * \internal (for Plugin)
     *
     * Tells the kontact core that a part has been loaded.
     */
    virtual void partLoaded(Plugin *plugin, KParts::Part *part) = 0;

Q_SIGNALS:
    /*!
     * This signal is emitted whenever a new day starts.
     *
     * \a date The date of the new day
     */
    void dayChanged(const QDate &date);

protected:
    /*!
     * Creates a new core object.
     *
     * \a parent The parent widget.
     * \a flags The window flags.
     */
    explicit Core(QWidget *parent = nullptr, Qt::WindowFlags flags = {});

    /*!
     * Returns the last error message for problems during
     * KParts loading.
     */
    QString lastErrorMessage() const;

private:
    friend class CorePrivate;
    std::unique_ptr<CorePrivate> const d;
};

}
