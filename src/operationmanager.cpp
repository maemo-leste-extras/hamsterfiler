#include "operationmanager.h"

OperationManager* OperationManager::instance = NULL;

OperationManager* OperationManager::get()
{
    return instance ? instance : instance = new OperationManager();
}

OperationManager::OperationManager()
{
    archiveSize = DEFAULT_ArchiveSize;
    activeLimit = DEFAULT_ActiveLimiter ? DEFAULT_MaxActive : -1;
}

void OperationManager::add(FileOperation *operation)
{
    connect(operation, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(operation, SIGNAL(progressChanged(int,QString,QString)), this, SLOT(onProgressChanged(int,QString,QString)));
    connect(operation, SIGNAL(overwriteSituation()), this, SLOT(onOverwriteSituation()));
    connect(operation, SIGNAL(selfOverwriteSituation()), this, SLOT(onSelfOverwriteSituation()));
    connect(operation, SIGNAL(errorSituation()), this, SLOT(onErrorSituation()));

    if (activeLimit == -1 || activeOperations.size() < activeLimit) {
        operation->start();
        activeOperations.prepend(operation);
    } else {
        queuedOperations.prepend(operation);
    }

    emit queueChanged();
}

FileOperation* OperationManager::findActiveById(int id)
{
    foreach (FileOperation *op, activeOperations)
        if (op->id == id)
            return op;

    return NULL;
}

void OperationManager::pause(int id)
{
    FileOperation *operation = findActiveById(id);
    if (operation) operation->pause();
}

void OperationManager::resume(int id)
{
    FileOperation *operation = findActiveById(id);
    if (operation) {
        operation->resume();
    } else {
        // Check also the queued operations
        for (int i = 0; i < queuedOperations.size(); i++) {
            if (queuedOperations.at(i)->id == id) {
                activeOperations.prepend(queuedOperations.takeAt(i));
                activeOperations.first()->start();
                emit queueChanged();
            }
        }
    }
}

void OperationManager::abort(int id)
{
    FileOperation *operation = findActiveById(id);
    if (operation) {
        operation->abort();
    } else {
        // It should be possible to abort the queued operations too
        for (int i = 0; i < queuedOperations.size(); i++) {
            if (queuedOperations.at(i)->id == id) {
                archivalOperations.prepend(queuedOperations.takeAt(i));
                archivalOperations.first()->abort();
                emit queueChanged();
            }
        }
    }
}

void OperationManager::remove(int id)
{
    for (int i = 0; i < archivalOperations.size(); i++) {
        if (archivalOperations.at(i)->id == id) {
            delete archivalOperations.takeAt(i);
            emit queueChanged();
        }
    }
}

void OperationManager::setArchiveSize(int archiveSize)
{
    this->archiveSize = archiveSize;

    cleanArchive();

    emit queueChanged();
}

void OperationManager::setActiveLimit(int activeLimit)
{
    this->activeLimit = activeLimit;

    fillQueue();

    emit queueChanged();
}

void OperationManager::onStateChanged(int state)
{
    FileOperation *operation = qobject_cast<FileOperation*>(this->sender());

    emit stateChanged(operation->id, state);

    if (state == FileOperation::Aborted || state == FileOperation::Complete) {
        activeOperations.removeOne(operation);
        archivalOperations.prepend(operation);

        disconnect(operation, NULL, this, NULL);
        operation->purge();

        fillQueue();

        cleanArchive();

        emit queueChanged();
    }
}

void OperationManager::onProgressChanged(int progress, QString source, QString destination)
{
    emit progressChanged(qobject_cast<FileOperation*>(this->sender())->id, progress, source, destination);
}

void OperationManager::onOverwriteSituation()
{
    FileOperation *operation = qobject_cast<FileOperation*>(this->sender());
    emit overwriteSituation(operation->id, operation->type, operation->source(), operation->target());
}

void OperationManager::onSelfOverwriteSituation()
{
    FileOperation *operation = qobject_cast<FileOperation*>(this->sender());
    emit selfOverwriteSituation(operation->id, operation->type, operation->source(), operation->target());
}

void OperationManager::onErrorSituation()
{
    FileOperation *operation = qobject_cast<FileOperation*>(this->sender());
    emit errorSituation(operation->id, operation->type, operation->source(), operation->target());
}

void OperationManager::setAttentionAction(int id, FileOperation::Action action, QString hint)
{
    foreach (FileOperation *op, activeOperations) {
        if (op->id == id) {
            op->setAttentionAction(action, hint);
            break;
        }
    }
}

void OperationManager::fillQueue()
{
    while (!queuedOperations.isEmpty() && (activeLimit == -1 || activeOperations.size() < activeLimit)) {
        activeOperations.prepend(queuedOperations.takeLast());
        activeOperations.first()->start();
    }
}

void OperationManager::cleanArchive()
{
    while (archivalOperations.size() > archiveSize)
        delete archivalOperations.takeLast();
}

void OperationManager::clearArchive()
{
    while (!archivalOperations.isEmpty())
        delete archivalOperations.takeLast();

    emit queueChanged();
}
