#ifndef SIZECOUNTER_H
#define SIZECOUNTER_H

#include <QThread>

#include <QFileInfo>
#include <QDir>
#include <QTime>

class SizeCounter : public QThread
{
    Q_OBJECT

public:
    SizeCounter(const QFileInfo &root);
    void run();
    void abort();

Q_SIGNALS:
    void sizeUpdated(qint64 size, int dirs, int files);

private:
    QFileInfo root;
    QTime time;
    bool aborted;

    qint64 size;
    int dirs;
    int files;

    void calculate(const QFileInfo &root);
};

#endif // SIZECOUNTER_H
