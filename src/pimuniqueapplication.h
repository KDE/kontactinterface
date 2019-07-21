/* This file is part of the KDE project

   Copyright 2008 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KONTACTINTERFACE_PIMUNIQUEAPPLICATION_H
#define KONTACTINTERFACE_PIMUNIQUEAPPLICATION_H

#include "kontactinterface_export.h"

#include <QApplication>

class KAboutData;
class QCommandLineParser;

namespace KontactInterface
{

/**
 * KDEPIM applications which can be integrated into kontact should use
 * PimUniqueApplication instead of QApplication + Dbus unique.
 * This makes command-line handling work, i.e. you can launch "korganizer"
 * and if kontact is already running, it will load the korganizer part and
 * switch to it.
 */
class KONTACTINTERFACE_EXPORT PimUniqueApplication : public QApplication
{

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.PIMUniqueApplication")

public:
    explicit PimUniqueApplication(int &argc, char **argv[]);
    ~PimUniqueApplication();

    void setAboutData(KAboutData &aboutData);

    /**
     * Register this process as a unique application, if not already running.
     * Typically called in main().
     * @param arguments should start with the appname, as QCoreApplication::arguments() does.
     */
    static bool start(const QStringList &arguments);

    /**
     * Ensure that another PIM application is running.
     */
    static bool activateApplication(const QString &application,
                                    const QStringList &additionalArguments = {});

    Q_REQUIRED_RESULT QCommandLineParser *cmdArgs() const;

public Q_SLOTS:
    Q_SCRIPTABLE int newInstance();
    Q_SCRIPTABLE virtual int newInstance(const QByteArray &startupId, const QStringList &arguments, const QString &workingDirectory);

protected:
    virtual int activate(const QStringList &arguments, const QString &workingDirectory);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
