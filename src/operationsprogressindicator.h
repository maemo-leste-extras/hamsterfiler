#ifndef OPERATIONSPROGRESSINDICATOR_H
#define OPERATIONSPROGRESSINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QMaemo5Style>

#include "operationmanager.h"

class OperationsProgressIndicator : public QWidget
{
    Q_OBJECT

public:
    OperationsProgressIndicator(QWidget *parent = 0);

private:
    QList<int> idList;
    QList<int> progressList;

    void paintEvent(QPaintEvent *e);

private slots:
    void reload();
    void updateProgress(int id, int progress);
};

#endif // OPERATIONSPROGRESSINDICATOR_H
