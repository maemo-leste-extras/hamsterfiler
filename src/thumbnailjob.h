#ifndef THUMBNAILJOB_H
#define THUMBNAILJOB_H

#include <QStandardItem>

#include <hildon-thumbnail/hildon-thumbnail-factory.h>

class ThumbnailJob
{
public:
    ThumbnailJob(const gchar *uri, QStandardItem *item);
    ~ThumbnailJob();

    void start();
    void abort();

private:
    HildonThumbnailRequest *request;
    QStandardItem *item;
    gchar *uri;

    static void onThumbnailReady(HildonThumbnailFactory *, const gchar *uri, GError *, gpointer data);
};

#endif // THUMBNAILJOB_H
