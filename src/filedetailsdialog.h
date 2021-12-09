#ifndef FILEDETAILSDIALOG_H
#define FILEDETAILSDIALOG_H

#include <QDebug>

#include <QDialog>
#include <QDesktopWidget>
#include <QRegExpValidator>
#include <QFileInfo>
#include <QDateTime>
#include <QLabel>
#include <QMaemo5Style>
#include <QMaemo5InformationBox>

#include "ui_filedetailsdialog.h"
#include "filesystemmodel.h"
#include "sizecounter.h"

namespace Ui {
    class FileDetailsDialog;
}

class FileDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    FileDetailsDialog(QWidget *parent, QFileInfo file);
    ~FileDetailsDialog();

private:
    Ui::FileDetailsDialog *ui;
    SizeCounter *sizeCounter;
    QFileInfo info;

private slots:
    void onOrientationChanged();
    void onSizeStarted();
    void onSizeUpdated(qint64 size, int dirs = -1, int files = 0);
    void onSizeReady();
    void save();
};

#endif // FILEDETAILSDIALOG_H
