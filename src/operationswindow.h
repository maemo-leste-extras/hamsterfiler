#ifndef OPERATIONSWINDOW_H
#define OPERATIONSWINDOW_H

#include <QDebug>

#include <QMainWindow>
#include <QShortcut>

#include "ui_operationswindow.h"
#include "operationdelegate.h"
#include "operationmanager.h"

namespace Ui {
    class OperationsWindow;
}

class OperationsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit OperationsWindow(QWidget *parent = 0);
    ~OperationsWindow();

private:
    Ui::OperationsWindow *ui;

    int livingOperationsCount;

    QListWidgetItem* findOperationById(int id);

private slots:
    void listOperations();
    void showOperationMenu(const QPoint &pos);

    void onStateChanged(int id, int state);
    void onProgressChanged(int id, int progress, QString source, QString destination);

    void abortCurrentItem();
    void removeCurrentItem();

    void onItemActivated(QListWidgetItem *item);
    void onClearClicked();
};

#endif // OPERATIONSWINDOW_H
