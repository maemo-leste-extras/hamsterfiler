#ifndef CREATEDIALOG_H
#define CREATEDIALOG_H

#include <QDebug>

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QFile>
#include <QDir>
#include <QMaemo5InformationBox>

class CreateDialog : public QDialog
{
    Q_OBJECT

public:
    CreateDialog(QString currentPath, QWidget *parent);

private:
    QString currentPath;

    QLineEdit *nameEdit;
    QPushButton *fileButton;
    QPushButton *directoryButton;

private Q_SLOTS:
    void create();
};

#endif // CREATEDIALOG_H
