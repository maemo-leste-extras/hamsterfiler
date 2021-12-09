#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QDebug>

#include <QListView>
#include <QScrollBar>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QShortcut>
#include <QContextMenuEvent>
#include <QMaemo5InformationBox>

#include <hildon-mime.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "filesystemmodel.h"
#include "detaileddelegate.h"
#include "griddelegate.h"
#include "sharedialog.h"
#include "applicationswindow.h"
#include "filedetailsdialog.h"
#include "operationmanager.h"
#include "clipboard.h"
#include "confirmdialog.h"

class FileListView : public QListView
{
    Q_OBJECT

public:
    FileListView(QWidget *parent);

    QString currentPath();

    void setDetails(FileSystemModel::Detail firstDetail, FileSystemModel::Detail secondDetail);
    void setResumePosition(int resumePosition);

    int position();

public Q_SLOTS:
    void openPath(QString path);

    void enableHidden(bool enable);
    void enableThumbnails(bool enable);
    void enableLocationItem(bool enable);
    void enableMultiSelection(bool enable);
    void enableGridMode(bool enable);

    void sortByName();
    void sortByTime();
    void sortBySize();
    void sortByType();

    void clipSelected();
    void openSelected();
    void deleteSelected();

Q_SIGNALS:
    void locationChanged(QString path, int oldPosition);
    void loadingStarted();
    void loadingFinished();

private:
    void keyPressEvent(QKeyEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void resizeEvent(QResizeEvent *e);

    int toGridPosition(int listPosition);
    int fromGridPosition(int gridPosition);

    FileSystemModel *filesystemModel;
    int resumePosition;

private Q_SLOTS:
    void onActivated(QModelIndex index);
    void onLoadingStarted();
    void onLoadingFinished();
    void examineCurrentItem();
    void showShareDialog();
};

#endif // FILELISTVIEW_H
