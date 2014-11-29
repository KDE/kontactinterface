/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

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

#include "summary.h"

#include <QFont>
#include <QLabel>
#include <QDrag>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QHBoxLayout>

#include <KIconLoader>
#include <QFontDatabase>

using namespace KontactInterface;

//@cond PRIVATE
namespace KontactInterface
{
class SummaryMimeData : public QMimeData
{
public:
    virtual bool hasFormat(const QString &format) const
    {
        if (format == QLatin1String("application/x-kontact-summary")) {
            return true;
        }
        return false;
    }
};
}
//@endcond

//@cond PRIVATE
class Summary::Private
{
public:
    QPoint mDragStartPoint;
};
//@endcond

Summary::Summary(QWidget *parent)
    : QWidget(parent), d(new Private)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    setAcceptDrops(true);
}

Summary::~Summary()
{
    delete d;
}

int Summary::summaryHeight() const
{
    return 1;
}

QWidget *Summary::createHeader(QWidget *parent, const QString &iconname, const QString &heading)
{
    setStyleSheet(QLatin1String("KHBox {"
                                "border: 0px;"
                                "font: bold large;"
                                "padding: 2px;"
                                "background: palette(window);"
                                "color: palette(windowtext);"
                                "}"
                                "KHBox > QLabel { font: bold larger; } "));

    QWidget *box = new QWidget(parent);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    box->setLayout(hbox);

    QLabel *label = new QLabel(box);
    hbox->addWidget(label);
    label->setPixmap(KIconLoader::global()->loadIcon(iconname, KIconLoader::Toolbar));

    label->setFixedSize(label->sizeHint());
    label->setAcceptDrops(true);

    label = new QLabel(heading, box);
    hbox->addWidget(label);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//TODO PORT QT5   label->setIndent( QDialog::spacingHint() );

    box->setMaximumHeight(box->minimumSizeHint().height());

    return box;
}

QStringList Summary::configModules() const
{
    return QStringList();
}

void Summary::configChanged()
{
}

void Summary::updateSummary(bool force)
{
    Q_UNUSED(force);
}

void Summary::mousePressEvent(QMouseEvent *event)
{
    d->mDragStartPoint = event->pos();

    QWidget::mousePressEvent(event);
}

void Summary::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
            (event->pos() - d->mDragStartPoint).manhattanLength() > 4) {

        QDrag *drag = new QDrag(this);
        drag->setMimeData(new SummaryMimeData());
        drag->setObjectName(QLatin1String("SummaryWidgetDrag"));

        QPixmap pm = QPixmap::grabWidget(this);
        if (pm.width() > 300) {
            pm = QPixmap::fromImage(
                     pm.toImage().scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        QPainter painter;
        painter.begin(&pm);
        painter.setPen(QPalette::AlternateBase);
        painter.drawRect(0, 0, pm.width(), pm.height());
        painter.end();
        drag->setPixmap(pm);
        drag->start(Qt::MoveAction);
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void Summary::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QLatin1String("application/x-kontact-summary"))) {
        event->acceptProposedAction();
    }
}

void Summary::dropEvent(QDropEvent *event)
{
    int alignment = (event->pos().y() < (height() / 2) ? Qt::AlignTop : Qt::AlignBottom);
    emit summaryWidgetDropped(this, event->source(), alignment);
}

