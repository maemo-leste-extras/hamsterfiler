#ifndef DETAILEDDELEGATE_H
#define DETAILEDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QMaemo5Style>

#include "filesystemmodel.h"

class DetailedDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DetailedDelegate(QObject *parent) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, QModelIndex());

        QRect r = option.rect;
        QFont f = painter->font();
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

        painter->save();

        painter->drawPixmap(r.left()+3, r.top()+11, 48, 48, qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(48,48));
        r.setLeft(r.left() + 3+48+3);

        f.setItalic(index.data(FileSystemModel::SymLinkRole).toBool());
        f.setStrikeOut(index.data(FileSystemModel::BrokenRole).toBool());
        f.setPointSize(18);
        painter->setFont(f);
        painter->drawText(r.left(), r.top()+7, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, index.data(Qt::DisplayRole).toString());

        f.setStrikeOut(false);
        f.setItalic(false);
        f.setPointSize(13);
        painter->setFont(f);
        painter->setPen(QPen(secondaryColor));
        painter->drawText(r.left(), r.top(), r.width(), r.height()-7, Qt::AlignBottom|Qt::AlignLeft, index.data(FileSystemModel::FirstDetailRole).toString());
        painter->drawText(r.left(), r.top(), r.width()-7, r.height()-7, Qt::AlignBottom|Qt::AlignRight, index.data(FileSystemModel::SecondDetailRole).toString());

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(400, 70);
    }
};

#endif // DETAILEDDELEGATE_H
