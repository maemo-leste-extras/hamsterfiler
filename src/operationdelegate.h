#ifndef OPERATIONDELEGATE_H
#define OPERATIONDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QProgressBar>
#include <QMaemo5Style>

#include "fileoperation.h"

class OperationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum {
        IdRole = Qt::UserRole,
        TypeRole,
        StateRole,
        ProgressRole,
        SourceRole,
        TargetRole
    };

    OperationDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, QModelIndex());

        QRect r = option.rect;
        QFont f = painter->font();
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

        int state = index.data(StateRole).toInt();
        QIcon icon = QIcon::fromTheme(state == FileOperation::Initial   ? "general_clock" :
                                      state == FileOperation::Preparing ? "camera_playback" :
                                      state == FileOperation::Running   ? "camera_playback" :
                                      state == FileOperation::Pausing   ? "camera_video_pause" :
                                      state == FileOperation::Paused    ? "camera_video_pause" :
                                      state == FileOperation::Aborting  ? "general_stop" :
                                      state == FileOperation::Aborted   ? "general_stop" :
                                                                          "widgets_tickmark_list");

        QString source = index.data(SourceRole).toString();
        QString target = index.data(TargetRole).toString();

        QString sourceLabel = FileOperation::labelForType(static_cast<FileOperation::Type>(index.data(TypeRole).toInt()));

        painter->save();

        painter->drawPixmap(r.left()+3, r.top()+5, 48, 48, icon.pixmap(48,48));

        f.setPointSize(13);
        painter->setFont(f);
        QFontMetrics fm(f);
        int labelWidth = qMax(fm.width(sourceLabel), fm.width(sourceLabel));

        source = fm.elidedText(source, Qt::ElideLeft, r.width()-(3+48+4)-labelWidth-6-6);
        target = fm.elidedText(target, Qt::ElideLeft, r.width()-(3+48+4)-labelWidth-6-6);

        // Details
        painter->drawText(r.left()+(3+48+3)+labelWidth+6, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, source);
        if (!target.isEmpty())
            painter->drawText(r.left()+(3+48+3)+labelWidth+6, r.top(), r.width(), r.height()-3-6-6, Qt::AlignBottom|Qt::AlignLeft, target);

        painter->setPen(QPen(secondaryColor));

        // Labels
        painter->drawText(r.left()+(3+48+3), r.top()+5, labelWidth, r.height(), Qt::AlignTop|Qt::AlignRight, sourceLabel);
        if (!target.isEmpty())
            painter->drawText(r.left()+(3+48+3), r.top(), labelWidth, r.height()-3-6-6, Qt::AlignBottom|Qt::AlignRight, tr("To"));

        // Progress bar
        float progress = index.data(ProgressRole).toInt() / 100.0f;
        painter->drawRect(r.left()+10, r.bottom()-3-6, r.width()-(10+10), 6);
        painter->fillRect(r.left()+10, r.bottom()-3-6, (r.width()-(10+10))*progress, 6, secondaryColor);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(400, 70);
    }
};

#endif // OPERATIONDELEGATE_H
