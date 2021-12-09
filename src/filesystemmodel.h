#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QDebug>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QFileSystemWatcher>
#include <QDir>
#include <QDateTime>
#include <QTimer>

#include <QThread>

#include <sys/stat.h>
#include <hildon-thumbnail/hildon-thumbnail-factory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <gio/gio.h>

#include "thumbnailjob.h"

class FileSystemModelWorker;

class FileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT

    friend class FileSystemModelWorker;

public:
    enum {
        PathRole = Qt::UserRole,
        SymLinkRole,
        BrokenRole,
        FirstDetailRole,
        SecondDetailRole
    };

    enum Detail {
        Nothing = 0,
        Size,
        Date,
        Permissions
    };

    static const int ThumbnailSize = 48;

    FileSystemModel(QObject *parent);

    QString location();
    void setLocation(QString path);
    void setDetails(Detail firstDetail, Detail secondDetail);

    void enableDotDot(bool enable);
    void enableHidden(bool enable);
    void enableThumbnails(bool enable);

    void sortByName();
    void sortByTime();
    void sortBySize();
    void sortByType();

    QIcon thumbnail(QFileInfo info, QStandardItem *item);
    static QIcon icon(QFileInfo info);
    static QString sizeString(double size);
    static QString dateString(QDateTime date);
    static QString detailString(QFileInfo info, Detail detail);

signals:
    void loadingStarted();
    void loadingFinished();

private:
    QStandardItemModel *model;
    QFileSystemWatcher *watcher;

    bool showDotDot;
    bool showHidden;
    bool showThumbnails;
    void rebuildFilter();

    QDir currentDir;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    Detail firstDetail;
    Detail secondDetail;

    FileSystemModelWorker *currentWorker;
    QList<FileSystemModelWorker*> workers;
    QList<ThumbnailJob*> thumbnailJobs;
    QList<QStandardItem*> itemBuffer;

    int currentThumbnailJob;

private slots:
    void reload();
    void processThumbnailJob();
    void onItemReady(QStandardItem *item);
    void onListReady();
    void onItemUpdated(int index, int role, QString value);
    void onWorkerFinished();
};

class FileSystemModelWorker : public QThread
{
    Q_OBJECT

public:
    FileSystemModelWorker(FileSystemModel *owner);
    void run();

signals:
    void itemReady(QStandardItem*);
    void listReady();
    void itemUpdated(int index, int role, QString value);

private:
    QDir currentDir;
    FileSystemModel::Detail firstDetail;
    FileSystemModel::Detail secondDetail;

    void updateSize(const QFileInfoList &entries, int role);
};

#endif // FILESYSTEMMODEL_H
