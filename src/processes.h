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
 * Activates window for first found process with executable @a executableName
 * (without path and .exe extension)
 */
KONTACTINTERFACE_EXPORT void activateWindowForProcess(const QString &executableName);
#endif

}
