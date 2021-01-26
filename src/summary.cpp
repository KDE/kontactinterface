/*
  This file is part of the KDE Kontact Plugin Interface Library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "summary.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFont>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

using namespace KontactInterface;

//@cond PRIVATE
namespace KontactInterface
{
class SummaryMimeData : public QMimeData
{
    Q_OBJECT
public:
    bool hasFormat(const QString &format) const override
    {
        return format == QLatin1String("application/x-kontact-summary");
    }
};
}
//@endcond

//@cond PRIVATE
class Q_DECL_HIDDEN Summary::Private
{
public:
    QPoint mDragStartPoint;
};
//@endcond

Summary::Summary(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
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
    QWidget *box = new QWidget(parent);
    QHBoxLayout *hbox = new QHBoxLayout(box);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    box->setAutoFillBackground(true);

    QIcon icon = QIcon::fromTheme(iconname);

    QLabel *label = new QLabel(box);
    hbox->addWidget(label);
    label->setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_ToolBarIconSize)));

    label->setFixedSize(label->sizeHint());
    label->setAcceptDrops(true);

    label = new QLabel(heading, box);
    hbox->addWidget(label);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

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
    Q_UNUSED(force)
}

void Summary::mousePressEvent(QMouseEvent *event)
{
    d->mDragStartPoint = event->pos();

    QWidget::mousePressEvent(event);
}

void Summary::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (event->pos() - d->mDragStartPoint).manhattanLength() > 4) {
        QDrag *drag = new QDrag(this);
        drag->setMimeData(new SummaryMimeData());
        drag->setObjectName(QStringLiteral("SummaryWidgetDrag"));

        QPixmap pm = grab();
        if (pm.width() > 300) {
            pm = QPixmap::fromImage(pm.toImage().scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        QPainter painter;
        painter.begin(&pm);
        painter.setPen(QPalette::AlternateBase);
        painter.drawRect(0, 0, pm.width(), pm.height());
        painter.end();
        drag->setPixmap(pm);
        drag->exec(Qt::MoveAction);
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void Summary::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-kontact-summary"))) {
        event->acceptProposedAction();
    }
}

void Summary::dropEvent(QDropEvent *event)
{
    const int alignment = (event->pos().y() < (height() / 2) ? Qt::AlignTop : Qt::AlignBottom);
    Q_EMIT summaryWidgetDropped(this, event->source(), alignment);
}

#include <summary.moc>
