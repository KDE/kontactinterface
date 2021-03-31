/**
 * This file is part of the kpimutils library.
 *
 * SPDX-FileCopyrightText: 2008 Jarosław Staniek <staniek@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
/**
  @file
  This file is part of the KDEPIM Utilities library and provides
  static methods for process handling.

  @brief
  Process handling methods.

  @author Jarosław Staniek \<staniek@kde.org\>
*/

#pragma once

#include "kontactinterface_export.h"

#include <QList>

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

