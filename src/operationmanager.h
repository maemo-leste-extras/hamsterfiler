#ifndef OPERATIONMANAGER_H
#define OPERATIONMANAGER_H

#include <QDebug>

#include <QObject>

#include "global.h"

#include "fileoperation.h"

class OperationManager : public QObject
{
    Q_OBJECT

public:
    static OperationManager* get();

    void add(FileOperation *operation);
    void pause(int id);
    void resume(int id);
    void abort(int id);
    void remove(int id);
    void setAttentionAction(int id, FileOperation::Action, QString hint = QString());

    void setActiveLimit(int activeLimit);
    void setArchiveSize(int archiveSize);
    void clearArchive();

    QList<FileOperation*> activeOperations;
    QList<FileOperation*> queuedOperations;
    QList<FileOperation*> archivalOperations;

Q_SIGNALS:
    void stateChanged(int id, int state);
    void progressChanged(int id, int progress, QString source, QString target);
    void queueChanged();

    void overwriteSituation(int id, FileOperation::Type type, QString source, QString target);
    void selfOverwriteSituation(int id, FileOperation::Type type, QString source, QString target);
    void errorSituation(int id, FileOperation::Type type, QString source, QString target);

private:
    OperationManager();
    static OperationManager *instance;

    FileOperation* findActiveById(int id);

    void cleanArchive();
    int archiveSize;

    void fillQueue();
    int activeLimit;

private Q_SLOTS:
    void onStateChanged(int state);
    void onProgressChanged(int progress, QString source, QString target);

    void onOverwriteSituation();
    void onSelfOverwriteSituation();
    void onErrorSituation();
};

#endif // OPERATIONMANAGER_H
