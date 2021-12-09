#ifndef DESCRIPTIVEDELEGATE_H
#define DESCRIPTIVEDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QMaemo5Style>

class DescriptiveDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum {
        DescriptionRole = Qt::UserRole
    };

    DescriptiveDelegate(QObject *parent) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, QModelIndex());

        QRect r = option.rect;
        QFont f = painter->font();
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        if (!icon.isNull()) {
            painter->drawPixmap(r.left()+3, r.top()+11, 48, 48, icon.pixmap(48,48));
            r.setLeft(r.left() + 3+48+3);
        } else {
            r.setLeft(r.left() + 7);
        }

        f.setPointSize(18);
        painter->setFont(f);
        painter->drawText(r.left(), r.top()+7, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, index.data(Qt::DisplayRole).toString());

        f.setPointSize(13);
        painter->setFont(f);
        painter->setPen(QPen(secondaryColor));
        painter->drawText(r.left(), r.top(), r.width(), r.height()-7, Qt::AlignBottom|Qt::AlignLeft, index.data(DescriptionRole).toString());

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return QSize(400, 70);
    }
};

#endif // DESCRIPTIVEDELEGATE_H
