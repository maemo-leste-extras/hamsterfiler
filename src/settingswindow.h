#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDebug>

#include <QMainWindow>
#include <QSettings>
#include <QMaemo5ListPickSelector>
#include <QStandardItemModel>
#include <QShortcut>

#include "global.h"

#include "ui_settingswindow.h"
#include "filesystemmodel.h"
#include "settingswindow.h"

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QString currentPath, QWidget *parent);
    ~SettingsWindow();

private:
    Ui::SettingsWindow *ui;

    QString currentPath;

    void load();
    void save();

private Q_SLOTS:
    void setStartingLocation();
};

#endif // SETTINGSWINDOW_H
