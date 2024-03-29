/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "core.h"
#include "kontactinterface_debug.h"

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QDateTime>
#include <QTimer>

using namespace KontactInterface;

//@cond PRIVATE
class Q_DECL_HIDDEN KontactInterface::CorePrivate
{
    Core *const q;

public:
    explicit CorePrivate(Core *qq);

    void slotPartDestroyed(QObject *);
    void checkNewDay();

    QString lastErrorMessage;
    QDate mLastDate;
    QMap<QByteArray, KParts::Part *> mParts;
};

CorePrivate::CorePrivate(Core *qq)
    : q(qq)
    , mLastDate(QDate::currentDate())
{
}
//@endcond

Core::Core(QWidget *parent, Qt::WindowFlags f)
    : KParts::MainWindow(parent, f)
    , d(new CorePrivate(this))
{
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        d->checkNewDay();
    });
    timer->start(1000 * 60);
}

Core::~Core() = default;

KParts::Part *Core::createPart(const char *libname)
{
    qCDebug(KONTACTINTERFACE_LOG) << libname;

    QMap<QByteArray, KParts::Part *>::ConstIterator it;
    it = d->mParts.constFind(libname);
    if (it != d->mParts.constEnd()) {
        return it.value();
    }

    qCDebug(KONTACTINTERFACE_LOG) << "Creating new KPart";
    const auto result = KPluginFactory::instantiatePlugin<KParts::Part>(KPluginMetaData(QString::fromLatin1(libname)), this);
    if (result.plugin) {
        d->mParts.insert(libname, result.plugin);
        QObject::connect(result.plugin, &KParts::Part::destroyed, this, [this](QObject *obj) {
            d->slotPartDestroyed(obj);
        });
    } else {
        d->lastErrorMessage = result.errorString;
        qCWarning(KONTACTINTERFACE_LOG) << d->lastErrorMessage;
    }
    return result.plugin;
}

//@cond PRIVATE
void CorePrivate::slotPartDestroyed(QObject *obj)
{
    // the part was deleted, we need to remove it from the part map to not return
    // a dangling pointer in createPart
    const QMap<QByteArray, KParts::Part *>::Iterator end = mParts.end();
    QMap<QByteArray, KParts::Part *>::Iterator it = mParts.begin();
    for (; it != end; ++it) {
        if (it.value() == obj) {
            mParts.erase(it);
            return;
        }
    }
}

void CorePrivate::checkNewDay()
{
    if (mLastDate != QDate::currentDate()) {
        Q_EMIT q->dayChanged(QDate::currentDate());
    }

    mLastDate = QDate::currentDate();
}
//@endcond

QString Core::lastErrorMessage() const
{
    return d->lastErrorMessage;
}

#include "moc_core.cpp"
