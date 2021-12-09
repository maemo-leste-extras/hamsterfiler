#ifndef CLIPBOARDWINDOW_H
#define CLIPBOARDWINDOW_H

#include <QDebug>

#include <QMainWindow>
#include <QMaemo5InformationBox>
#include <QShortcut>

#include "ui_clipboardwindow.h"
#include "descriptivedelegate.h"
#include "filesystemmodel.h"
#include "clipboard.h"
#include "operationmanager.h"
#include "confirmdialog.h"

namespace Ui {
    class ClipboardWindow;
}

class ClipboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClipboardWindow(QString currentPath, QWidget *parent);
    ~ClipboardWindow();

private:
    Ui::ClipboardWindow *ui;

    QString currentPath;

    void listFiles();
    int syncSelection();

private slots:
    void onUnclipClicked();
    void onClearClicked();

    void onCopyClicked();
    void onMoveClicked();
    void onLinkClicked();
    void onRemoveClicked();
};

#endif // CLIPBOARDWINDOW_H
