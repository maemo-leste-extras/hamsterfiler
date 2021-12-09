#ifndef GRIDDELEGATE_H
#define GRIDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QMaemo5Style>

#include "filesystemmodel.h"

class GridDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    GridDelegate(QObject *parent) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, QModelIndex());

        QRect r = option.rect;
        QFont f = painter->font();
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

        painter->save();

        painter->drawPixmap(r.left()+(r.width()-48)/2, r.top(), 48, 48, qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(48,48));

        f.setItalic(index.data(FileSystemModel::SymLinkRole).toBool());
        f.setStrikeOut(index.data(FileSystemModel::BrokenRole).toBool());
        f.setPointSize(13);

        painter->setFont(f);
        painter->drawText(r.left()+3, r.top(), r.width()-6, r.height()-5, Qt::AlignBottom|Qt::AlignHCenter,
                          QFontMetrics(f).elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, r.width()-6));

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(140, 79);
    }
};

#endif // GRIDDELEGATE_H
