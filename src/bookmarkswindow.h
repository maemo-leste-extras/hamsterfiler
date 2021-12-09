#ifndef BOOKMARKSWINDOW_H
#define BOOKMARKSWINDOW_H

#include <QDebug>

#include <QMainWindow>
#include <QSettings>
#include <QMaemo5InformationBox>
#include <QShortcut>

#include "ui_bookmarkswindow.h"
#include "descriptivedelegate.h"
#include "filesystemmodel.h"
#include "confirmdialog.h"

namespace Ui {
    class BookmarksWindow;
}

class BookmarksWindow : public QMainWindow
{
    Q_OBJECT

public:
    BookmarksWindow(QString currentPath, QWidget *parent);
    ~BookmarksWindow();

Q_SIGNALS:
    void locationSelected(QString path);

private:
    Ui::BookmarksWindow *ui;

    QString currentPath;

    void updateVisibility();

private Q_SLOTS:
    void onItemActivated(QListWidgetItem *item);
    void showBookmarkMenu(const QPoint &pos);

    void onAddClicked();
    void onClearClicked();

    void renameCurrentBookmark();
    void removeCurrentBookmark();

    void save();
};

#endif // BOOKMARKSWINDOW_H
