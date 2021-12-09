#ifndef LOCATIONWIDGET_H
#define LOCATIONWIDGET_H

#include <QDebug>

#include <QScrollArea>
#include <QScrollBar>
//#include <QAbstractKineticScroller>
#include <QVariant>
#include <QAbstractButton>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QCompleter>
#include <QFileSystemModel>
#include <QApplication>
#include <QHBoxLayout>
#include <QStringList>
#include <QList>
#include <QSettings>

#include "global.h"

class LocationWidget : public QScrollArea
{
    Q_OBJECT

public:
    LocationWidget(QWidget *parent);
    ~LocationWidget();

    int setLocation(QString path, int oldPosition);
    bool isEditorEnabled();
    void reloadSettings();

public Q_SLOTS:
    void enableEditor();
    void disableEditor();
    void emitLocationFromEditor();

Q_SIGNALS:
    void locationSelected(QString path);

private:
    void resizeEvent(QResizeEvent *e);

    bool isEditorPersistent();

    QAbstractButton* createButton(QString name);
    QAbstractButton *checkedButton;
    void centerCheckedButton();

    QWidget *buttonWidget;
    QWidget *editorWidget;
    QToolButton *locationUpButton;
    QLineEdit *locationEdit;

    QString currentPath;

    QList<int> scrollPositions;

private Q_SLOTS:
    void onButtonClicked();
    void goUp();
};

#endif // LOCATIONWIDGET_H
