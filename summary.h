/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KONTACTINTERFACE_SUMMARY_H
#define KONTACTINTERFACE_SUMMARY_H

#include "kontactinterface_export.h"

#include <QtGui/QWidget>

class KStatusBar;
class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;

namespace KontactInterface
{

/**
 * @short Base class for summary widgets in Kontact.
 *
 * This class should be used as base class when creating new
 * summary widgets for the Summary View plugin in Kontact.
 */
class KONTACTINTERFACE_EXPORT Summary : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Creates a new summary widget.
     *
     * @param parent The parent widget.
     */
    explicit Summary( QWidget *parent );

    /**
     * Destroys the summary widget.
     */
    virtual ~Summary();

    /**
     * Returns the logical height of summary widget.
     *
     * This is used to calculate how much vertical space relative
     * to other summary widgets this widget will use in the summary view.
     */
    virtual int summaryHeight() const;

    /**
     * Creates a heading for a typical summary view with an icon and a heading.
     *
     * @param parent The parent widget.
     * @param icon The name of the icon.
     * @param heading The text of the header.
     */
    QWidget *createHeader( QWidget *parent, const QString &icon, const QString &heading );

    /**
     * Returns a list of names identifying configuration modules for this summary widget.
     *
     * @note The names have to be suitable for being passed to KCMultiDialog::addModule().
     */
    virtual QStringList configModules() const;

  public Q_SLOTS:
    /**
     * This method is called whenever the configuration has been changed.
     */
    virtual void configChanged();

    /**
     * This method is called if the displayed information should be updated.
     *
     * @param force true if the update was requested by the user
     */
    virtual void updateSummary( bool force = false );

  Q_SIGNALS:
    /**
     * This signal can be emitted to signaling that an action is going on.
     * The @p message will be shown in the status bar.
     */
    void message( const QString &message );

    /**
     * @internal
     *
     * This signal is emitted whenever a summary widget has been dropped on
     * this summary widget.
     */
    void summaryWidgetDropped( QWidget *target, QWidget *widget, int alignment );

  protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dropEvent( QDropEvent * );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
