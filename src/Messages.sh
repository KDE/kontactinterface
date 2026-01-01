#! /bin/sh
# SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/kontactinterfaces6.pot
