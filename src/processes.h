/**
 * This file is part of the kpimutils library.
 *
 * Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
/**
  @file
  This file is part of the KDEPIM Utilities library and provides
  static methods for process handling.

  @brief
  Process handling methods.

  @author Jarosław Staniek \<staniek@kde.org\>
*/

#ifndef KONTACTINTERFACE_PROCESSES_H
#define KONTACTINTERFACE_PROCESSES_H

#include "kontactinterface_export.h"

#include <QtCore/QList>

class QString;

namespace KontactInterface
{

#ifdef Q_OS_WIN
/**
 * Sets @a pids to a list of processes having name @a processName.
 */
KONTACTINTERFACE_EXPORT void getProcessesIdForName(const QString &processName, QList<int> &pids);

/**
 * @return true if one or more processes (other than the current process) exist
 * for name @a processName; false otherwise.
 */
KONTACTINTERFACE_EXPORT bool otherProcessesExist(const QString &processName);

/**
 * Terminates or kills all processes with name @a processName.
 * First, SIGTERM is sent to a process, then if that fails, we try with SIGKILL.
 * @return true on successful termination of all processes or false if at least
 *         one process failed to terminate.
 */
KONTACTINTERFACE_EXPORT bool killProcesses(const QString &processName);

/**
 * Activates window for first found process with executable @a executableName
 * (without path and .exe extension)
 */
KONTACTINTERFACE_EXPORT void activateWindowForProcess(const QString &executableName);
#endif

}

#endif

