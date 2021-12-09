#ifndef APPLICATIONSWINDOW_H
#define APPLICATIONSWINDOW_H

#include <QDebug>

#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <QShortcut>
#include <QMenuBar>
#include <QMenu>

#include "ui_applicationswindow.h"
#include "descriptivedelegate.h"
#include "applicationdialog.h"
#include "confirmdialog.h"

namespace Ui {
    class ApplicationsWindow;
}

class ApplicationsWindow : public QMainWindow
{
    Q_OBJECT

public:
    ApplicationsWindow(QWidget *parent, QStringList files = QStringList());
    ~ApplicationsWindow();

private:
    Ui::ApplicationsWindow *ui;

    QStringList files;

    void updateVisibility();
    void save();

private Q_SLOTS:
    void onItemActivated(QListWidgetItem *item);
    void showApplicationMenu(const QPoint &pos);

    void onAddClicked();
    void onClearClicked();

    void editCurrentApplication();
    void removeCurrentApplication();
};

#endif // APPLICATIONSWINDOW_H
