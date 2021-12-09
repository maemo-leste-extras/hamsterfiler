#include "filelistview.h"

// Item sizes
#define GRID_WIDTH 142
#define GRID_HEIGHT 80
#define LIST_HEIGHT 70

FileListView::FileListView(QWidget *parent) :
    QListView(parent)
{
    this->setUniformItemSizes(true);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setMovement(QListView::Static);

    enableGridMode(false);
    resumePosition = 0;

    // Initialize the model
    filesystemModel = new FileSystemModel(this);
    this->setModel(filesystemModel);

    connect(filesystemModel, SIGNAL(loadingStarted()), this, SLOT(onLoadingStarted()));
    connect(filesystemModel, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
}

void FileListView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Enter:
            QListView::keyPressEvent(e); break;

        case Qt::Key_Left:
            if (this->viewMode() == QListView::IconMode)
                QListView::keyPressEvent(e);
            else if (currentPath() != "/")
                openPath("..");
            break;

        case Qt::Key_Right:
            if (this->viewMode() == QListView::IconMode)
                QListView::keyPressEvent(e);
            else if (QFileInfo(this->currentIndex().data(FileSystemModel::PathRole).toString()).isDir())
                onActivated(this->currentIndex());
            break;

        default:
            e->ignore();
    }
}

void FileListView::enableGridMode(bool enable)
{
    int convertedPosition = 0;

    if (enable) {
        if (this->viewMode() == QListView::ListMode)
            convertedPosition = toGridPosition(this->verticalScrollBar()->value());

        this->itemDelegate()->deleteLater();
        this->setItemDelegate(new GridDelegate(this));

        this->setViewMode(QListView::IconMode);
        this->setGridSize(QSize(GRID_WIDTH, GRID_HEIGHT));
        this->setFlow(QListView::LeftToRight);
    } else {
        if (this->viewMode() == QListView::IconMode)
            convertedPosition = fromGridPosition(this->verticalScrollBar()->value());

        this->itemDelegate()->deleteLater();
        this->setItemDelegate(new DetailedDelegate(this));

        this->setViewMode(QListView::ListMode);
        this->setGridSize(QSize());
        this->setFlow(QListView::TopToBottom);
    }

    if (convertedPosition) {
        // Wait for the list to be resized
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        this->verticalScrollBar()->setValue(convertedPosition);
    }
}

int FileListView::position()
{
    return this->viewMode() == QListView::IconMode ?
           fromGridPosition(this->verticalScrollBar()->value()) :
           this->verticalScrollBar()->value();
}

int FileListView::toGridPosition(int listPosition)
{
    return listPosition
         * ((double) GRID_HEIGHT / LIST_HEIGHT)
         / (this->width() / GRID_WIDTH); // items per row
}

int FileListView::fromGridPosition(int gridPosition)
{
    return gridPosition
         * ((double) LIST_HEIGHT / GRID_HEIGHT)
         * (this->width() / GRID_WIDTH); // items per row
}

void FileListView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
    this->setFlow(this->flow());
}

void FileListView::onActivated(QModelIndex index)
{
    QString path = index.data(FileSystemModel::PathRole).toString();

    if (!path.endsWith(".."))
        resumePosition = -1; // ignore the saved position

    openPath(path);
}

void FileListView::onLoadingStarted()
{
    // Remember the position to restore aftrer repopulating the list
    resumePosition = position();

    Q_EMIT loadingStarted();
}

void FileListView::onLoadingFinished()
{
    // Wait for the list to be resized
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // Restore the posiotion from before repopulating
    this->verticalScrollBar()->setValue(this->viewMode() == QListView::IconMode ?
                                        toGridPosition(resumePosition) :
                                        resumePosition);

    Q_EMIT loadingFinished();
}

QString FileListView::currentPath()
{
    return filesystemModel->location();
}

// Choose which details should be displayed
void FileListView::setDetails(FileSystemModel::Detail firstDetail, FileSystemModel::Detail secondDetail)
{
    filesystemModel->setDetails(firstDetail, secondDetail);
}

void FileListView::setResumePosition(int resumePosition)
{
    this->resumePosition = this->resumePosition == -1 ? 0 : resumePosition;
}

void FileListView::openPath(QString path)
{
    if (!path.startsWith("/"))
        path = filesystemModel->location() + '/' + path;

    QFileInfo file(path);
    path = file.absoluteFilePath();

    if (!file.exists()) {
        qDebug() << "File does not exist" << path;
        return;
    }

    if (file.isDir()) {
        qDebug() << "Changing directory" << path;

        filesystemModel->setLocation(path);

        Q_EMIT locationChanged(path, position());

        this->clearSelection();
        this->scrollToTop();
    } else {
        qDebug() << "Opening file" << path;
        gchar *uri = g_filename_to_uri(path.toUtf8(), nullptr, nullptr);
        hildon_mime_open_file(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, nullptr)), uri);
        g_free(uri);
    }
}

void FileListView::contextMenuEvent(QContextMenuEvent *e)
{
    this->selectionModel()->select(this->currentIndex(), QItemSelectionModel::SelectCurrent);

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    contextMenu->addAction(tr("Details"), this, SLOT(examineCurrentItem()));
    contextMenu->addAction(tr("Clip"), this, SLOT(clipSelected()));
    contextMenu->addAction(tr("Open with..."), this, SLOT(openSelected()));
    /*contextMenu->addAction(tr("Copy to..."), this, SLOT(copySelected()));
    contextMenu->addAction(tr("Move to..."), this, SLOT(moveSelected()));
    contextMenu->addAction(tr("Link to..."), this, SLOT(linkSelected()));*/
    contextMenu->addAction(tr("Share"), this, SLOT(showShareDialog()));
    contextMenu->addAction(tr("Delete"), this, SLOT(deleteSelected()));

    contextMenu->exec(e->globalPos());
}

void FileListView::enableHidden(bool enable)
{
    filesystemModel->enableHidden(enable);
}

void FileListView::enableThumbnails(bool enable)
{
    filesystemModel->enableThumbnails(enable);
}

void FileListView::enableLocationItem(bool enable)
{
    filesystemModel->enableDotDot(enable);
}

void FileListView::enableMultiSelection(bool enable)
{
    if (enable) {
        disconnect(this, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
        this->setSelectionMode(QAbstractItemView::MultiSelection);
        this->setSelectionRectVisible(true);
    } else {
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setSelectionRectVisible(false);
        connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)), Qt::UniqueConnection);
    }

    this->clearSelection();
}

void FileListView::sortByName()
{
    QSettings().setValue("Browser/Sorting", QDir::Name);
    filesystemModel->sortByName();
}

void FileListView::sortByTime()
{
    QSettings().setValue("Browser/Sorting", QDir::Time);
    filesystemModel->sortByTime();
}

void FileListView::sortBySize()
{
    QSettings().setValue("Browser/Sorting", QDir::Size);
    filesystemModel->sortBySize();
}

void FileListView::sortByType()
{
    QSettings().setValue("Browser/Sorting", QDir::Type);
    filesystemModel->sortByType();
}

void FileListView::examineCurrentItem()
{
    FileDetailsDialog(this, this->currentIndex().data(FileSystemModel::PathRole).toString()).exec();
}

void FileListView::clipSelected()
{
    QItemSelectionModel *selection = this->selectionModel();

    if (selection->hasSelection()) {
        QFileInfoList files;
        QModelIndexList indices = selection->selectedIndexes();
        for(QModelIndex index: indices)
            files.append(index.data(FileSystemModel::PathRole).toString());

        Clipboard::get()->add(files);
        QMaemo5InformationBox::information(this, tr("Added %n item(s) to clipboard", "", files.size()));
    } else {
        QMaemo5InformationBox::information(this, tr("Selection is empty"));
    }
}

void FileListView::openSelected()
{
    QStringList files;
    QModelIndexList indices = this->selectionModel()->selectedIndexes();
    for(QModelIndex index: indices)
            files.append(index.data(FileSystemModel::PathRole).toString());

    ApplicationsWindow *applications = new ApplicationsWindow(this, files);
    applications->show();
}

void FileListView::deleteSelected()
{
    if (ConfirmDialog(this, ConfirmDialog::DeleteSelected).exec() == QMessageBox::Yes) {
        QFileInfoList files;
        QModelIndexList indices = this->selectionModel()->selectedIndexes();
        for(QModelIndex index: indices)
            files.append(index.data(FileSystemModel::PathRole).toString());

        OperationManager::get()->add(new FileOperation(FileOperation::Remove, files));
    }
}

void FileListView::showShareDialog()
{
    // Build a list of files to share, exclude directories
    QStringList files;
    QModelIndexList indices = this->selectionModel()->selectedIndexes();
    for(QModelIndex index: indices) {
        QFileInfo info(index.data(FileSystemModel::PathRole).toString());
        if (info.isFile())
            files.append(info.absoluteFilePath());
    }

    if (files.isEmpty())
        QMaemo5InformationBox::information(this, tr("There are no files to share"));
    else
        ShareDialog(files, this).exec();
}
