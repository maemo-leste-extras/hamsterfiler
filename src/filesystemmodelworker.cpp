#include "filesystemmodel.h"

FileSystemModelWorker::FileSystemModelWorker(FileSystemModel *owner)
{
    currentDir = owner->currentDir;
    firstDetail = owner->firstDetail;
    secondDetail = owner->secondDetail;
}

void FileSystemModelWorker::FileSystemModelWorker::run()
{
    currentDir.refresh();
    QFileInfoList entries = currentDir.entryInfoList();
    foreach (QFileInfo entry, entries) {
        QStandardItem *item = new QStandardItem();

        item->setText(entry.fileName());
        item->setData(entry.absoluteFilePath(), FileSystemModel::PathRole);

        item->setData(FileSystemModel::detailString(entry, firstDetail), FileSystemModel::FirstDetailRole);
        item->setData(FileSystemModel::detailString(entry, secondDetail), FileSystemModel::SecondDetailRole);
        item->setData(entry.isSymLink(), FileSystemModel::SymLinkRole);
        item->setData(!entry.exists(), FileSystemModel::BrokenRole);

        emit itemReady(item);
    }

    emit listReady();

    if (firstDetail == FileSystemModel::Size)
        updateSize(entries, FileSystemModel::FirstDetailRole);
    if (secondDetail == FileSystemModel::Size)
        updateSize(entries, FileSystemModel::SecondDetailRole);
}

// Second stage of size calculation
void FileSystemModelWorker::updateSize(const QFileInfoList &entries, int role)
{
    QDir dir;
    dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    for (int i = 0; i < entries.size(); i++)
        // NOTE: The documentation incorrectly states that QDir::cd() returns
        // false if the directory is not readable. A separate check is required.
        if (entries.at(i).isDir()
        &&  entries.at(i).isReadable()
        &&  dir.cd(entries.at(i).absoluteFilePath()))
            emit itemUpdated(i, role, tr("%n item(s)", "", dir.count()));
}
