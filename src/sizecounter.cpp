#include "sizecounter.h"

SizeCounter::SizeCounter(const QFileInfo &root)
{
    this->root = root;

    aborted = false;

    size = 0;
    dirs = -1;
    files = 0;
}

void SizeCounter::run()
{
    time.start();

    calculate(root);

    emit sizeUpdated(size, dirs, files);
}

void SizeCounter::abort()
{
    aborted = true;
}

void SizeCounter::calculate(const QFileInfo &root)
{
    size += root.size();

    // Do not emit updates too often
    if (time.elapsed() > 1000) {
        emit sizeUpdated(size, dirs, files);
        time.restart();
    }

    if (root.isDir()) {
        dirs++;

        // Do not descend into symlinks
        if (!root.isSymLink()) {
            if (aborted) return;

            foreach (QFileInfo entry, QDir(root.absoluteFilePath()).entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot))
                calculate(entry);
        }
    } else {
        files++;
    }
}
