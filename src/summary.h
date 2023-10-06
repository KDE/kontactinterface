/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kontactinterface_export.h"

#include <QWidget>

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
    explicit Summary(QWidget *parent);

    /**
     * Destroys the summary widget.
     */
    ~Summary() override;

    /**
     * Returns the logical height of summary widget.
     *
     * This is used to calculate how much vertical space relative
     * to other summary widgets this widget will use in the summary view.
     */
    [[nodiscard]] virtual int summaryHeight() const;

    /**
     * Creates a heading for a typical summary view with an icon and a heading.
     *
     * @param parent The parent widget.
     * @param icon The name of the icon.
     * @param heading The text of the header.
     */
    [[nodiscard]] QWidget *createHeader(QWidget *parent, const QString &icon, const QString &heading);

    /**
     * Returns a list of names identifying configuration modules for this summary widget.
     *
     * @note The names have to be suitable for being passed to KCMultiDialog::addModule().
     */
    [[nodiscard]] virtual QStringList configModules() const;

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
    virtual void updateSummary(bool force = false);

Q_SIGNALS:
    /**
     * This signal can be emitted to signaling that an action is going on.
     * The @p message will be shown in the status bar.
     */
    void message(const QString &message);

    /**
     * @internal
     *
     * This signal is emitted whenever a summary widget has been dropped on
     * this summary widget.
     */
    void summaryWidgetDropped(QWidget *target, QObject *object, int alignment);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

private:
    //@cond PRIVATE
    class SummaryPrivate;
    std::unique_ptr<SummaryPrivate> const d;
    //@endcond
};

}
