#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDebug>

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

class ApplicationDialog : public QDialog
{
    Q_OBJECT

public:
    ApplicationDialog(QWidget *parent, QString name = QString(), QString command = QString());

    QLineEdit *nameEdit;
    QLineEdit *commandEdit;
};

#endif // APPLICATIONDIALOG_H
