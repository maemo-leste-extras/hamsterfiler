#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QDebug>

#include <QThread>
#include <QMutex>
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include <unistd.h>

// A structure to represent parts of operations requiring data copying
struct FileJob {
    QString source;
    QString target;
    qint64 size;
    bool isLink;
    bool isDir;
};

class FileOperation : public QThread
{
    Q_OBJECT

public:
    enum Type {
        Copy,
        Move,
        Link,
        Remove
    };

    enum State {
        Initial,
        Preparing,
        Running,
        Pausing,
        Paused,
        Aborting,
        Aborted,
        Complete
    };

    enum Action {
        Ask,
        Allow,
        AllowAll,
        Skip,
        SkipAll,
        Merge,
        Retry,
        Abort
    };

    int id;
    Type type;

    FileOperation(Type type, QFileInfoList files, QFileInfo targetDir = QFileInfo());
    void run();

    int progress();
    State state();
    QString source();
    QString target();

    // Progress control
    void pause();
    void resume();
    void abort();

    void setAttentionAction(Action action, QString hint = QString());

    void purge();

    static QString labelForType(Type type);

signals:
    void progressChanged(int progress, QString source, QString target);
    void stateChanged(int state);

    void overwriteSituation();
    void selfOverwriteSituation();
    void errorSituation();

private:
    // A counter to give each operation a unique ID
    static int idCounter;

    // Synchronization
    QMutex pauseMutex; // operation freeze
    QMutex memberMutex; // member modification
    QMutex attentionMutex; // user intervention

    // To be used by the actual operation
    QFileInfo targetDir;
    QFileInfoList files;
    QList<FileJob> jobs;
    qint64 totalSize;

    // State variables, with thread-safe getters and setters
    State state_m;
    int progress_m;
    QString source_m;
    QString target_m;

    QString attentionHint;
    Action attentionAction;
    Action overwriteAction;
    Action selfOverwriteAction;
    Action errorAction;

    Action checkOverwrite(QFileInfo source, QFileInfo target);
    Action checkError(bool success);

    void reportState(State state);
    void reportProgress(int progress, QString source, QString target);

    bool rest();
    void runCopy();
    void runMove();
    void runCopyOrMove();
    void runLink();
    void runRemove();

    void silentRemove(QString path);

    QString deepCopy(QString);
};

#endif // FILEOPERATION_H
