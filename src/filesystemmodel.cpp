#include "filesystemmodel.h"

FileSystemModel::FileSystemModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    this->setSourceModel(model = new QStandardItemModel(this));
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(reload()));

    currentWorker = NULL;

    firstDetail = Size;
    secondDetail = Date;

    showThumbnails = false;
    showDotDot = false;
    enableHidden(false);
}

bool FileSystemModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return model->index(sourceRow, 0, sourceParent).data(Qt::DisplayRole).toString() == ".." ||
           QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

QString FileSystemModel::location()
{
    return currentDir.absolutePath();
}

void FileSystemModel::setLocation(QString path)
{
    QStringList watchDirs = watcher->directories();
    if (!watchDirs.isEmpty())
        watcher->removePaths(watchDirs);

    currentDir.cd(path);
    watcher->addPath(currentDir.absolutePath());

    reload();
}

void FileSystemModel::setDetails(Detail firstDetail, Detail secondDetail)
{
    this->firstDetail = firstDetail;
    this->secondDetail = secondDetail;

    reload();
}

void FileSystemModel::reload()
{
    // Avoid pointless reloading before the first path is set
    if (currentDir.path() == ".") return;

    emit loadingStarted();

    // Clear thumbnail jobs
    foreach (ThumbnailJob *job, thumbnailJobs)
        job->abort();
    thumbnailJobs.clear();
    currentThumbnailJob = 0;

    // Clear buffered items
    foreach (QStandardItem *item, itemBuffer)
        delete item;
    itemBuffer.clear();

    model->clear();

    if (showDotDot && currentDir.path() != "/") {
        QStandardItem *item = new QStandardItem();
        item->setText("..");
        item->setIcon(QIcon::fromTheme("filemanager_folder_up"));
        item->setData(currentDir.absolutePath() + "/..", PathRole);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        model->appendRow(item);
    }

    if (currentDir.exists()) {
        FileSystemModelWorker *worker = new FileSystemModelWorker(this);
        connect(worker, SIGNAL(itemReady(QStandardItem*)), this, SLOT(onItemReady(QStandardItem*)));
        connect(worker, SIGNAL(listReady()), this, SLOT(onListReady()));
        connect(worker, SIGNAL(itemUpdated(int,int,QString)), this, SLOT(onItemUpdated(int,int,QString)));
        connect(worker, SIGNAL(finished()), this, SLOT(onWorkerFinished()));

        if (currentWorker)
            workers.prepend(currentWorker);

        currentWorker = worker;
        worker->start();
    } else {
        emit loadingFinished();
    }
}

void FileSystemModel::onItemReady(QStandardItem *item)
{
    // Only results from the most recent job are interesting
    if (this->sender() == currentWorker) {
        item->setIcon(showThumbnails
                      ? thumbnail(item->data(PathRole).toString(), item)
                      : icon(item->data(PathRole).toString()));
        itemBuffer.append(item);
    } else {
        delete item;
    }
}

void FileSystemModel::onListReady()
{
    if (this->sender() == currentWorker) {
        foreach (QStandardItem *item, itemBuffer)
            model->appendRow(item);
        itemBuffer.clear();

        QTimer::singleShot(0, this, SLOT(processThumbnailJob()));

        emit loadingFinished();
    }
}

void FileSystemModel::processThumbnailJob()
{
    if (currentThumbnailJob < thumbnailJobs.count())
        thumbnailJobs.at(currentThumbnailJob++)->start();

    if (currentThumbnailJob < thumbnailJobs.count())
        QTimer::singleShot(0, this, SLOT(processThumbnailJob()));
}

void FileSystemModel::onItemUpdated(int index, int role, QString detail)
{
    if (this->sender() == currentWorker)
        model->item(index, 0)->setData(detail, role);
}

void FileSystemModel::onWorkerFinished()
{
    if (this->sender() == currentWorker) {
        currentWorker = NULL;
    } else {
        workers.removeOne(static_cast<FileSystemModelWorker*>(this->sender()));
    }

    delete this->sender();
}

void FileSystemModel::enableDotDot(bool enable)
{
    if (showDotDot != enable && currentDir.path() != "/") {
        if (enable) {
            QStandardItem *item = new QStandardItem();
            item->setText("..");
            item->setIcon(QIcon::fromTheme("filemanager_folder_up"));
            item->setData(currentDir.absolutePath() + "/..", PathRole);
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            model->insertRow(0, item);
        } else {
            model->removeRow(0);
        }
    }

    showDotDot = enable;
}

void FileSystemModel::enableHidden(bool enable)
{
    showHidden = enable;
    rebuildFilter();
}

void FileSystemModel::enableThumbnails(bool enable)
{
    showThumbnails = enable;
    reload();
}

void FileSystemModel::rebuildFilter()
{
    QDir::Filters oldFilters = currentDir.filter();
    QDir::Filters newFilters = QDir::AllEntries | QDir::AllDirs | QDir::System | QDir::NoDotAndDotDot;

    if (showHidden) newFilters |= QDir::Hidden;

    if (newFilters != oldFilters) {
        currentDir.setFilter(newFilters);
        reload();
    }
}

void FileSystemModel::sortByName()
{
    currentDir.setSorting(QDir::DirsFirst | QDir::LocaleAware | QDir::IgnoreCase | QDir::Name);
    reload();
}

void FileSystemModel::sortByTime()
{
    currentDir.setSorting(QDir::DirsFirst | QDir::LocaleAware | QDir::IgnoreCase | QDir::Time);
    reload();
}

void FileSystemModel::sortBySize()
{
    currentDir.setSorting(QDir::DirsFirst | QDir::LocaleAware | QDir::IgnoreCase | QDir::Size);
    reload();
}

void FileSystemModel::sortByType()
{
    currentDir.setSorting(QDir::DirsFirst | QDir::LocaleAware | QDir::IgnoreCase | QDir::Type);
    reload();
}

// Return a thumbnail for the given file, if available, or queue creation
QIcon FileSystemModel::thumbnail(QFileInfo info, QStandardItem *item)
{
    if (gchar* uri = g_filename_to_uri(info.absoluteFilePath().toUtf8(), NULL, NULL)) {
        if (gchar* file = hildon_thumbnail_get_uri(uri, ThumbnailSize, ThumbnailSize, true)) {
            // Simplified conversion of a URI (but apparently sufficient)
            QString thumbFile = QString::fromUtf8(file).mid(7); // "file://"
            g_free(file);

            if (QFileInfo(thumbFile).exists()) {
                // Thumbnail exists, return it immediately
                g_free(uri);
                return QIcon(thumbFile);
            } else if (!info.isDir()) {
                // Request thumbnail creation (but do not waste time on directories)
                thumbnailJobs.append(new ThumbnailJob(uri, item));
            }
        }
        g_free(uri);
    }

    // Fall back to an icon
    return icon(info);
}

// Return an icon for the given file
QIcon FileSystemModel::icon(QFileInfo info)
{
        if (info.isDir()) {
            if (info.fileName() == "..")
                return QIcon::fromTheme("filemanager_folder_up");
            else
                return QIcon::fromTheme("general_folder");
        } else {
            gchar *g_mime = g_content_type_guess(info.fileName().toUtf8(), NULL, 0, NULL);
            QString mime = QString(g_mime);
            g_free(g_mime);

            return QIcon::fromTheme("gnome-mime-" + mime.replace('/', '-'),
                   QIcon::fromTheme(mime.replace('/', '-'),
                   QIcon::fromTheme("filemanager_unknown_file")));
        }
}

QString FileSystemModel::sizeString(double size)
{
    const qint64 K = 1024;
    const qint64 M = 1024 * K;
    const qint64 G = 1024 * M;

    if (size >= G)
        return QString("%1 GiB").arg(size/G, 0, 'f', 1);
    else if (size >= M)
        return QString("%1 MiB").arg(size/M, 0, 'f', 1);
    else if (size >= K)
        return QString("%1 KiB").arg(size/K, 0, 'f', 1);
    else
        return QString("%1 B").arg(size);
}

QString FileSystemModel::dateString(QDateTime date)
{
    return QLocale().toString(date, "ddd d MMM yyyy, hh:mm:ss");
}

QString FileSystemModel::detailString(QFileInfo info, Detail detail)
{
    switch (detail) {
        default:
        case Nothing:
            return QString();

        case Size:
            return info.isDir() ? tr("Directory") : sizeString(info.size());

        case Date:
            return dateString(info.lastModified());

        case Permissions:
            char perms[9];
            struct stat fileStat;
            stat(info.absoluteFilePath().toUtf8(), &fileStat);

            perms[0] = fileStat.st_mode & S_IRUSR ? 'r' : '-';
            perms[1] = fileStat.st_mode & S_IWUSR ? 'w' : '-';
            perms[2] = fileStat.st_mode & S_ISUID ?
                      (fileStat.st_mode & S_IXUSR ? 's' : 'S'):
                      (fileStat.st_mode & S_IXUSR ? 'x' : '-');

            perms[3] = fileStat.st_mode & S_IRGRP ? 'r' : '-';
            perms[4] = fileStat.st_mode & S_IWGRP ? 'w' : '-';
            perms[5] = fileStat.st_mode & S_ISGID ?
                      (fileStat.st_mode & S_IXGRP ? 's' : 'S'):
                      (fileStat.st_mode & S_IXGRP ? 'x' : '-');

            perms[6] = fileStat.st_mode & S_IROTH ? 'r' : '-';
            perms[7] = fileStat.st_mode & S_IWOTH ? 'w' : '-';
            perms[8] = fileStat.st_mode & S_ISVTX ?
                      (fileStat.st_mode & S_IXOTH ? 't' : 'T'):
                      (fileStat.st_mode & S_IXOTH ? 'x' : '-');

            return info.owner() + ":" + info.group() + " " + QString::fromUtf8(perms, 9);
    }
}
