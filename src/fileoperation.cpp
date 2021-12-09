#include "fileoperation.h"

#define BUFFER_SIZE 16384

int FileOperation::idCounter = 0;

FileOperation::FileOperation(Type type, QFileInfoList files, QFileInfo targetDir)
{
    id = idCounter++;

    progress_m = 0;
    state_m = Initial;

    overwriteAction = Ask;
    selfOverwriteAction = Ask;
    errorAction = Ask;

    this->type = type;
    this->files = files;
    this->targetDir = targetDir;

    reportProgress(0, tr("%n item(s)", "", files.size()), targetDir.absoluteFilePath());
}

// Free operation details
void FileOperation::purge()
{
    if (state() < Aborted) return;

    files.clear();
    jobs.clear();
}

// Start the appropriate worker thread
void FileOperation::run()
{
    reportState(Preparing);

    // Exapnd the simple source list to a list which contains all files and directories
    QList<FileJob> expansionStack;
    totalSize = 1; // Prevent division by zero

    // Create an initial list for further expansion
    for (int i = 0; i < files.size(); i++) {
        FileJob job;
        job.source = files.at(i).absoluteFilePath();
        job.target = targetDir.absoluteFilePath() + '/' + files.at(i).fileName();
        expansionStack.prepend(job);
    }

    // Expand the item list to create an index of all files and directories
    while (!expansionStack.isEmpty()) {
        // Take one of the jobs to expand
        FileJob job = expansionStack.takeFirst();
        QFileInfo sourceInfo(job.source);

        if (sourceInfo.isSymLink()) {
            job.isLink = true;
            job.isDir = sourceInfo.isDir();
        } else if (sourceInfo.isFile()) {
            job.isLink = false;
            job.isDir = false;
        } else if (sourceInfo.isDir()) {
            job.isLink = false;
            job.isDir = true;

            // Add entries from the directory
            for(QString &entry: QDir(job.source).entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot)) {
                FileJob subJob;
                subJob.source = job.source + '/' + entry;
                subJob.target = job.target + '/' + entry;
                expansionStack.prepend(subJob);
            }
        }

        // Add to the final list
        job.size = sourceInfo.size(); // Symlinks should rather use a different value
        totalSize += job.size;
        jobs.append(job);
    }

    switch (type) {
        case Copy:
        case Move: runCopyOrMove(); break;
        case Link: runLink(); break;
        case Remove: runRemove(); break;
    }
}

// Return operation's progress
int FileOperation::progress()
{
    memberMutex.lock();
    int progress = progress_m;
    memberMutex.unlock();
    return progress;
}

// Remember and notify of progress changes
void FileOperation::reportProgress(int progress, QString source, QString target)
{
    memberMutex.lock();

    bool changed = false;

    if (progress_m != progress) {
        progress_m = progress;
        changed = true;
    }

    if (source_m != source) {
        source_m = deepCopy(source);
        changed = true;
    }

    if (target_m != target) {
        target_m = deepCopy(target);
        changed = true;
    }

    memberMutex.unlock();

    if (changed)
        Q_EMIT progressChanged(progress, source, target);
}

// Return operation's state
FileOperation::State FileOperation::state()
{
    memberMutex.lock();
    State state = state_m;
    memberMutex.unlock();
    return state;
}

// Remember and notify of state changes
void FileOperation::reportState(State state)
{
    memberMutex.lock();
    if (state_m == state) {
        memberMutex.unlock();
    } else {
        state_m = state;
        memberMutex.unlock();

        if (state == Complete)
            if (jobs.size() == 1)
                reportProgress(100, jobs.first().source, targetDir.absoluteFilePath());
            else
                reportProgress(100, tr("%n item(s)", "", jobs.size()), targetDir.absoluteFilePath());
        Q_EMIT stateChanged(state);
    }
}

// Return operation's current source
QString FileOperation::source()
{
    memberMutex.lock();
    QString source = deepCopy(source_m);
    memberMutex.unlock();
    return source;
}

// Return operation's current destination
QString FileOperation::target()
{
    memberMutex.lock();
    QString target = deepCopy(target_m);
    memberMutex.unlock();
    return target;
}

// Temporarily interrupt the operation
void FileOperation::pause()
{
    if (state() > Running) return;

    reportState(Pausing);
    pauseMutex.tryLock();
}

// Interrupt the operation
void FileOperation::abort()
{
    reportState(Aborting);
    pauseMutex.unlock();
}

// Allow the operation to continue
void FileOperation::resume()
{
    pauseMutex.unlock();
}

// Deliver feedback from the user
void FileOperation::setAttentionAction(Action action, QString hint)
{
    if (!attentionMutex.tryLock()) {
        attentionAction = action;
        attentionHint = hint;
    }
    attentionMutex.unlock();
}

// Detect and handle overwriting situations
FileOperation::Action FileOperation::checkOverwrite(QFileInfo source, QFileInfo target)
{
    qDebug() << "Examining overwrite:" << source.filePath() << "->" << target.filePath();

    if (target.exists() || target.isSymLink()) {
        // Prevent copying a file over itself
        if (source.filePath() == target.filePath() // File overwritten by itself
        || !target.isSymLink() && source.canonicalFilePath() == target.canonicalFilePath()) { // Link's source overwritten by the link
            switch (selfOverwriteAction) {
                case Skip:
                    // Skip this file
                    return Skip;

                case Ask: default:
                    // Ask the user what to do
                    attentionMutex.lock();
                    Q_EMIT selfOverwriteSituation();

                    // Wait for the user
                    attentionMutex.lock();
                    attentionMutex.unlock();

                    // Check user's decision
                    switch (attentionAction) {
                        case AllowAll:
                            // Waiting for automatic renaming
                        case Allow:
                            // Check for overwrite again
                            return Retry;

                        case SkipAll:
                            selfOverwriteAction = Skip;
                        case Skip:
                            return Skip;

                        case Ask:
                            pause();
                            return Retry;

                        case Abort:
                            abort();
                            return Retry;

                        default:
                            // Should never happen
                            return Retry;
                    }
            }
        }

        // Do not make unnecessary changes to the structure of directories
        if (source.isDir() && target.isDir())
            return Merge;

        // Decide what to do if the file already exists
        switch (overwriteAction) {
            case Allow:
                // Remove the file and grant a clearance
                silentRemove(target.absoluteFilePath());
                return Allow;

            case Skip:
                return Skip;

            case Ask: default:
                // Ask the user what to do
                attentionMutex.lock();
                Q_EMIT overwriteSituation();

                // Wait for the user
                attentionMutex.lock();
                attentionMutex.unlock();

                // Check user's decision
                switch (attentionAction) {
                    case AllowAll:
                        overwriteAction = Allow;
                    case Allow:
                        // Remove the file and grant a clearance
                        if (attentionHint.isNull()) {
                            silentRemove(target.absoluteFilePath());
                            return Allow;
                        } else {
                            // Check for another name collision
                            return Retry;
                        }

                    case SkipAll:
                        overwriteAction = Skip;
                    case Skip:
                        return Skip;

                    case Ask:
                        pause();
                        return Retry;

                    case Abort:
                        abort();
                        return Retry;

                    default:
                        // Should never happen
                        return Retry;
                }
        }
    } else {
        // No file, no problem
        return Allow;
    }
}

// Detect and handle error situations
FileOperation::Action FileOperation::checkError(bool success)
{
    if (success) {
        return Allow;
    } else {
        // Ask the user what to do
        attentionMutex.lock();
        Q_EMIT errorSituation();

        // Wait for the user
        attentionMutex.lock();
        attentionMutex.unlock();

        // Check user's decision
        switch (attentionAction) {
            case SkipAll:
                // Remember the decision
                errorAction = Skip;
            case Skip:
                // Skip the problematic file
                return Skip;

            case Ask:
                pause();
                return Retry;

            case Abort:
                abort();
                return Retry;

            default:
                return Retry;
        }
    }
}

// Handle state change requests and return the permission to continue
bool FileOperation::rest()
{
    if (!pauseMutex.tryLock()) {
        reportState(Paused);
        pauseMutex.lock();
    }
    pauseMutex.unlock();

    if (state() == Aborting)
        return false;
    else {
        reportState(Running);
        return true;
    }
}

// Perform copying or moving
void FileOperation::runCopyOrMove()
{
    char *buffer = new char[BUFFER_SIZE];
    qint64 transferredSize = 0;
    int iJob = 0;

    QList<FileJob*> remainderDirs;

    while (iJob < jobs.size()) {
        FileJob *job = &jobs[iJob];
        // Check if the operation can keep going
        if (rest()) {
            // Tell the world what is going on
            reportProgress(100 * transferredSize / totalSize, job->source, job->target);

            // Check if the file should be renamed
            if (!attentionHint.isNull()) {
                QString rootToRename = job->target + '/';
                job->target = attentionHint;
                attentionHint.append('/');

                for (int i = iJob+1; i < jobs.size() && jobs.at(i).target.startsWith(rootToRename); i++)
                    jobs[i].target.replace(0, rootToRename.length(), attentionHint);

                attentionHint = QString();
            }

            // Take a possibility of overwriting into account
            switch (checkOverwrite(job->source, job->target)) {
                case Allow: {
                    // We are allowed to use the target filename
                    if (job->isLink) {
                        QString linkTarget = QString::fromUtf8(buffer, readlink(job->source.toUtf8(), buffer, BUFFER_SIZE));
                        switch (checkError(QFile(linkTarget).link(job->target))) {
                            case Allow: {
                                // Remove the old link
                                if (type == Move)
                                    QFile(job->source).remove();
                            }
                            case Skip: {
                                // Move on
                                transferredSize += job->size;
                                iJob++;
                            }
                            default: {
                                // Process the job again
                                break;
                            }
                        }
                    }
                    else if (job->isDir) {
                        if (type == Move && QDir().rename(job->source, job->target)) {
                            // Move-by-rename succeeded, skip all jobs expanded from that directory
                            while (iJob < jobs.size() && jobs.at(iJob).source.startsWith(job->source)) {
                                transferredSize += jobs.at(iJob).size;
                                iJob++;
                            }
                        } else {
                            // Move-by-rename failed or copy mode, try to create a new directory
                            switch (checkError(QDir().mkdir(job->target))) {
                                case Allow: {
                                    // Move on to the next item
                                    transferredSize += job->size;
                                    remainderDirs.prepend(job);
                                    iJob++;
                                    break;
                                }
                                case Skip: {
                                    // Skip all items expanded from this directory
                                    while (iJob < jobs.size() && jobs.at(iJob).source.startsWith(job->source)) {
                                        transferredSize += jobs.at(iJob).size;
                                        iJob++;
                                    }
                                    break;
                                }
                                default: {
                                    // Let the loop handle Retry and Abort
                                    break;
                                }
                            }
                        }
                    } else {
                        if (type == Move && QFile(job->source).rename(job->target)) {
                            // Move-by-rename succeeded, move on to the next file
                            transferredSize += job->size;
                            iJob++;
                        } else {
                            // Move-by-rename failed or copy mode, try to copy the file
                            QFile in(job->source);
                            QFile out(job->target);
                            qint64 bytesComplete = 0;

                            // Try to open the files
                            switch (checkError(in.open(QIODevice::ReadOnly) &&
                                               out.open(QIODevice::WriteOnly | QIODevice::Truncate))) {
                                case Allow: {
                                    // We are ready to start copying data between files
                                    qDebug() << "Starting copy loop";
                                    while (rest()) {
                                        reportProgress(100 * (transferredSize+bytesComplete) / totalSize, job->source, job->target);

                                        // Get data from one file, write it to another and check for errors
                                        qint64 bytesRead;
                                        Action action = checkError(
                                            (bytesRead = in.read(buffer, BUFFER_SIZE)) != -1 &&
                                            (bytesRead == out.write(buffer, bytesRead)));

                                        // Apply error checker's decision
                                        if (action == Allow) {
                                            // We are allowed to continue
                                            if (in.atEnd()) {
                                                // The copy is complete, so update the global progress and move to the next job
                                                if (type == Move)
                                                    QFile(job->source).remove();
                                                transferredSize += job->size;
                                                iJob++;
                                                break;
                                            } else {
                                                // Update local progress and keep copying
                                                bytesComplete += bytesRead;
                                            }
                                        } else if (action == Skip) {
                                            // Clean up the partial copy and move on to the next job
                                            out.remove();
                                            break;
                                        } else {
                                            // Whether it's Abort or Restart, the outer loop will take care of that
                                            break;
                                        }
                                    }
                                    qDebug() << "Exited copy loop";
                                    break;
                                }
                                case Skip: {
                                    // Skip to the next file
                                    qDebug() << "Skipping file";
                                    transferredSize += job->size;
                                    iJob++;
                                    break;
                                }
                                default: {
                                    // Retry this file
                                    qDebug() << "Will retry this file";
                                    break;
                                }
                            }

                            // Finalize the copy
                            in.close();
                            out.close();
                        }
                    }
                    break;
                }
                case Skip: {
                    // Skipping a directory should skip all its contents too
                    while (iJob < jobs.size() && jobs.at(iJob).source.startsWith(job->source)) {
                        transferredSize += jobs.at(iJob).size;
                        iJob++;
                    }
                    break;
                }
                case Merge: {
                    // Merging directories, no special action required
                    transferredSize += job->size;
                    remainderDirs.prepend(job);
                    iJob++;
                }
                default: {
                    break;
                }
            }
        } else {
            reportState(Aborted);
            return;
        }
    }

    delete[] buffer;

    // Clean up dirs after move-by-copy
    if (type == Move) {
        QDir dir;
        for(const FileJob *job: remainderDirs)
            dir.rmdir(job->source);
    }

    reportState(Complete);
}

// Perform linking
void FileOperation::runLink()
{
    int iJob = 0;

    while (iJob < jobs.size()) {
        FileJob *job = &jobs[iJob];
        if (rest()) {
            // Report progress
            reportProgress(100 * iJob / jobs.size(), job->source, job->target);

            // Check for overwrites
            switch (checkOverwrite(job->source, job->target)) {
                case Allow: {
                    // Try to link the file
                    switch (checkError(QFile(job->source).link(job->target))) {
                        case Allow:
                        case Skip: {
                            // Move on to the next file
                            iJob++;
                            break;
                        }
                        default: {
                            // Let the loop handle Retry and Abort
                            break;
                        }
                    }
                    break;
                }
                case Skip: {
                    // Move on to the next file without doing anything
                    iJob++;
                    break;
                }
                default: {
                    // Let the loop handle this
                    break;
                }
            }
        } else {
            reportState(Aborted);
            return;
        }
    }

    reportState(Complete);
}

// Perform removing
void FileOperation::runRemove()
{
    int iJob = jobs.size() - 1;

    while (iJob >= 0) {
        const FileJob *job = &jobs.at(iJob);
        if (rest()) {
            reportProgress(100 * (jobs.size()-1-iJob) / jobs.size(), job->source, QString());

            switch (checkError(job->isDir ? QDir().rmdir(job->source)
                                          : QFile(job->source).remove())) {
                case Allow:
                case Skip: {
                    // Move on to the next file
                    iJob--;
                    break;
                }
                default: {
                    // Let the loop handle Retry and Abort
                    break;
                }
            }
        } else {
            reportState(Aborted);
            return;
        }
    }

    reportState(Complete);
}

// A simple function to delete a whole tree of directories
void FileOperation::silentRemove(QString path)
{
    QFileInfo info(path);
    if (info.isDir() && !info.isSymLink()) {
        QDir dir(path);
        for(QFileInfo entry: dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot))
            silentRemove(entry.absoluteFilePath());
        dir.rmdir(path);
    } else {
        QFile(path).remove();
    }
}

// Bypass the implicit sharing
QString FileOperation::deepCopy(QString string)
{
    return QString(string.unicode());
}

// The central source of inforamtion for various labels
QString FileOperation::labelForType(Type type)
{
    return type == FileOperation::Copy ? tr("Copying") :
           type == FileOperation::Move ? tr("Moving") :
           type == FileOperation::Link ? tr("Linking") :
                                         tr("Deleting");
}
