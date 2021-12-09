#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QSettings>
#include <QStringList>
#include <QFileInfo>

#include "global.h"

class Clipboard
{
public:
    static Clipboard *get();
    ~Clipboard();

    QFileInfoList selectedFiles(bool remove = false);

    void add(QFileInfoList files);
    void select(int index, bool select);
    void remove(int index);

    QStringList contents();
    QList<bool> selection();

    void clear();
    void save();

private:
    static Clipboard *instance;

    QStringList contents_m;
    QList<bool> selection_m;

    Clipboard();
};

#endif // CLIPBOARD_H
