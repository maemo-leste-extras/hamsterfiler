#ifndef ATTENTIONDIALOG_H
#define ATTENTIONDIALOG_H

#include <QDebug>

#include <QDialog>
#include <QMaemo5Style>

#include "ui_attentiondialog.h"
#include "operationmanager.h"
#include "prefixvalidator.h"

namespace Ui {
    class AttentionDialog;
}

class AttentionDialog : public QDialog
{
    Q_OBJECT

public:
    enum Type {
        Overwrite,
        SelfOverwrite,
        Error
    };

    AttentionDialog(Type type, int id, FileOperation::Type, QString source, QString target, QWidget *parent);
    ~AttentionDialog();

private:
    Ui::AttentionDialog *ui;

    QString target;
    int operationId;

private slots:
    void onOverwriteTargetChanged(QString target);
    void onSelfOverwriteTargetChanged(QString target);

    void onButtonClicked();
};

#endif // ATTENTIONDIALOG_H
