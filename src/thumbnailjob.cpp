#include "thumbnailjob.h"

#include "filesystemmodel.h"

ThumbnailJob::ThumbnailJob(const gchar *uri, QStandardItem *item)
{
    this->uri = g_strdup(uri);
    this->item = item;
    request = nullptr;
}

ThumbnailJob::~ThumbnailJob()
{
    g_free(uri);
}

void ThumbnailJob::start()
{
    HildonThumbnailFactory *factory = hildon_thumbnail_factory_get_instance();
    request = hildon_thumbnail_factory_request_uri(factory, uri,
                                                   FileSystemModel::ThumbnailSize,
                                                   FileSystemModel::ThumbnailSize,
                                                   true, nullptr,
                                                   onThumbnailReady, this, nullptr);
    g_object_unref(factory);

    g_free(uri);
    uri = nullptr;
}

void ThumbnailJob::abort()
{
    if (request) {
        item = nullptr;
    } else {
        delete this;
    }
}

void ThumbnailJob::onThumbnailReady(HildonThumbnailFactory *, const gchar *uri, GError *, gpointer data)
{
    ThumbnailJob *job = static_cast<ThumbnailJob*>(data);

    g_object_unref(job->request);

    if (job->item) {
        if (uri) {
            QString thumbFile = QString::fromUtf8(uri).mid(7);

            if (QFileInfo(thumbFile).exists())
                job->item->setIcon(QIcon(thumbFile));
        }

        job->request = nullptr;
    } else {
        delete job;
    }
}

