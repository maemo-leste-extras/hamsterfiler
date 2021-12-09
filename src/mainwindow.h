#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>

#include <QMainWindow>

#include "global.h"

#include "ui_mainwindow.h"
#include "rotator.h"
#include "history.h"
#include "clipboard.h"
#include "operationmanager.h"

#include "settingswindow.h"
#include "operationswindow.h"
#include "applicationswindow.h"
#include "clipboardwindow.h"
#include "bookmarkswindow.h"

#include "createdialog.h"
#include "attentiondialog.h"
#include "confirmdialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QFileInfo startingLocation);
    ~MainWindow();

    bool eventFilter(QObject *obj, QEvent *e);

private:
    Ui::MainWindow *ui;
    Rotator *rotator;
    History history;

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *e);

    QMainWindow *findTopWindow();

private Q_SLOTS:
    void onOrientationChanged(int w, int h);
    void onLocationChanged(QString path, int oldPosition);
    void onLoadingStarted();
    void onLoadingFinished();

    void enableFullscreen(bool enable);
    void reloadSettings();

    void showSettings();
    void showOperations();
    void showApplications();
    void showClipboard();
    void showBookmarks();
    void showCreateDialog();

    void showOverwriteDialog(int id, FileOperation::Type, QString source, QString target);
    void showSelfOverwriteDialog(int id, FileOperation::Type, QString source, QString target);
    void showErrorDialog(int id, FileOperation::Type, QString source, QString target);

    void onSelectClicked();

    void onSearchTextChanged(QString text);
    void closeSearch();

    void goBack();
    void goForward();
};

#endif // MAINWINDOW_H
