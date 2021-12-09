#include "history.h"

#define HISTORY_SIZE 32

History::History()
{
    currentItem = -1;
}

void History::add(QString path)
{
    if (currentItem != -1 && items.at(currentItem).path == path) return;

    // The future has changed, remove all items after the current one
    while (currentItem+1 < items.size())
        items.removeLast();

    // Create the requested item and place it at the end
    HistoryItem item = { path, 0 };
    items.append(item);

    // Remove the oldest item if the list is above the limit
    if (items.size() > HISTORY_SIZE)
        items.removeFirst();

    currentItem = items.size()-1;
}

void History::setPosition(int position)
{
    if (!items.isEmpty())
        items[currentItem].position = position;
}

HistoryItem History::back()
{
    return currentItem > 0 ? items.at(--currentItem)
                           : HistoryItem();
}

HistoryItem History::forward()
{
    return currentItem+1 < items.size() ? items.at(++currentItem)
                                        : HistoryItem();
}
