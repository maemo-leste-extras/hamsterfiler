#ifndef HISTORY_H
#define HISTORY_H

#include <QList>
#include <QString>

struct HistoryItem
{
    QString path;
    int position;
};

class History
{
public:
    History();

    void add(QString path);
    void setPosition(int position);
    HistoryItem back();
    HistoryItem forward();

private:
    QList<HistoryItem> items;
    int currentItem;
};

#endif // HISTORY_H
