#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDebug>

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QUrl>

class ShareDialog : public QDialog
{
    Q_OBJECT

public:
    ShareDialog(QStringList files, QWidget *parent = 0);

    QStringList files;

private Q_SLOTS:
    void onEmailClicked();
    void onBluetoothClicked();
};

#endif // SHAREDIALOG_H
