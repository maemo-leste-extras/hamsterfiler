#include "operationsprogressindicator.h"

#define HEIGHT 4
#define MARGIN 2
#define SPACER 6

OperationsProgressIndicator::OperationsProgressIndicator(QWidget *parent) :
    QWidget(parent)
{
    this->setFixedHeight(MARGIN+HEIGHT+MARGIN);
    this->setStyleSheet("background: rgb(255,0,0);");

    connect(OperationManager::get(), SIGNAL(queueChanged()), this, SLOT(reload()));
    connect(OperationManager::get(), SIGNAL(progressChanged(int,int,QString,QString)), this, SLOT(updateProgress(int,int)));

    reload();
}

void OperationsProgressIndicator::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (idList.isEmpty()) return;

    int barWidth = (this->width() - (MARGIN+MARGIN) - SPACER*(idList.size()-1)) / idList.size();

    QPainter painter(this);

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

    for (int i = 0, x = MARGIN; i < idList.size(); i++) {
        float progress = progressList.at(i) / 100.0f;

        painter.setPen(QPen(secondaryColor));
        painter.drawRect(x, MARGIN, barWidth, HEIGHT);
        painter.fillRect(x, MARGIN, barWidth*progress, HEIGHT, secondaryColor);

        x += barWidth + SPACER;
    }
}

void OperationsProgressIndicator::reload()
{
    idList.clear();
    progressList.clear();

    for(FileOperation *op: OperationManager::get()->queuedOperations) {
        idList.prepend(op->id);
        progressList.prepend(op->progress());
    }

    for(FileOperation *op: OperationManager::get()->activeOperations) {
        idList.prepend(op->id);
        progressList.prepend(op->progress());
    }

    this->setHidden(idList.isEmpty());
    this->update();
}

void OperationsProgressIndicator::updateProgress(int id, int progress)
{
    for (int i = 0; i < idList.size(); i++) {
        if (idList.at(i) == id) {
            progressList[i] = progress;
            this->update();
            break;
        }
    }
}
