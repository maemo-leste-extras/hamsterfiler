#include "operationswindow.h"

OperationsWindow::OperationsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OperationsWindow)
{
    ui->setupUi(this);
#ifdef MAEMO
  setProperty("X-Maemo-StackedWindow", 1);
#endif
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->operationList->setItemDelegate(new OperationDelegate(this));

    // Menu actions
    connect(ui->clearAction, SIGNAL(triggered()), this, SLOT(onClearClicked()));
    connect(ui->operationList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->operationList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showOperationMenu(QPoint)));

    // Operation manager
    connect(OperationManager::get(), SIGNAL(stateChanged(int,int)), this, SLOT(onStateChanged(int,int)));
    connect(OperationManager::get(), SIGNAL(progressChanged(int,int,QString,QString)), this, SLOT(onProgressChanged(int,int,QString,QString)));
    connect(OperationManager::get(), SIGNAL(queueChanged()), this, SLOT(listOperations()));

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(close()));

    listOperations();
}

OperationsWindow::~OperationsWindow()
{
    delete ui;
}

QListWidgetItem* OperationsWindow::findOperationById(int id)
{
    for (int i = 0; i < ui->operationList->count(); i++) {
        QListWidgetItem *item = ui->operationList->item(i);
        if (item->data(OperationDelegate::IdRole).toInt() == id)
            return item;
    }

    return nullptr;
}

void OperationsWindow::onClearClicked()
{
    OperationManager::get()->clearArchive();
}

void OperationsWindow::listOperations()
{
    ui->operationList->clear();

    OperationManager *manager = OperationManager::get();

    livingOperationsCount = manager->queuedOperations.size() + manager->activeOperations.size();

    // List queued operations first
    for(FileOperation *op: manager->queuedOperations) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(OperationDelegate::IdRole, op->id);
        item->setData(OperationDelegate::TypeRole, op->type);
        item->setData(OperationDelegate::ProgressRole, op->progress());
        item->setData(OperationDelegate::StateRole, op->state());
        item->setData(OperationDelegate::SourceRole, op->source());
        item->setData(OperationDelegate::TargetRole, op->target());
        ui->operationList->addItem(item);
    }

    // Active operations second
    for(FileOperation *op: manager->activeOperations) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(OperationDelegate::IdRole, op->id);
        item->setData(OperationDelegate::TypeRole, op->type);
        item->setData(OperationDelegate::ProgressRole, op->progress());
        item->setData(OperationDelegate::StateRole, op->state());
        item->setData(OperationDelegate::SourceRole, op->source());
        item->setData(OperationDelegate::TargetRole, op->target());
        ui->operationList->addItem(item);
    }

    // Finally, archival operations
    for(FileOperation *op: manager->archivalOperations) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(OperationDelegate::IdRole, op->id);
        item->setData(OperationDelegate::TypeRole, op->type);
        item->setData(OperationDelegate::ProgressRole, op->progress());
        item->setData(OperationDelegate::StateRole, op->state());
        item->setData(OperationDelegate::SourceRole, op->source());
        item->setData(OperationDelegate::TargetRole, op->target());
        ui->operationList->addItem(item);
    }

    // Hide the list and show the big label if there are no operations
    if (ui->operationList->count() == 0) {
        ui->operationList->hide();
        ui->emptyInfo->show();
    } else {
        ui->emptyInfo->hide();
        ui->operationList->show();
    }
}

void OperationsWindow::showOperationMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->operationList->currentRow() < livingOperationsCount)
        contextMenu->addAction(tr("Abort"), this, SLOT(abortCurrentItem()));
    else
        contextMenu->addAction(tr("Remove"), this, SLOT(removeCurrentItem()));

    contextMenu->exec(this->mapToGlobal(pos));
}

void OperationsWindow::onStateChanged(int id, int state)
{
    findOperationById(id)->setData(OperationDelegate::StateRole, state);
}

void OperationsWindow::onProgressChanged(int id, int progress, QString source, QString target)
{
    QListWidgetItem *item = findOperationById(id);
    item->setData(OperationDelegate::ProgressRole, progress);
    item->setData(OperationDelegate::SourceRole, source);
    item->setData(OperationDelegate::TargetRole, target);
}

void OperationsWindow::onItemActivated(QListWidgetItem *item)
{
    ui->operationList->clearSelection();
    switch (item->data(OperationDelegate::StateRole).toInt()) {
        case FileOperation::Running:
            OperationManager::get()->pause(item->data(OperationDelegate::IdRole).toInt());
            break;

        case FileOperation::Initial:
        case FileOperation::Pausing:
        case FileOperation::Paused:
            OperationManager::get()->resume(item->data(OperationDelegate::IdRole).toInt());

        default:
            break;
    }
}

void OperationsWindow::abortCurrentItem()
{
    ui->operationList->clearSelection();
    OperationManager::get()->abort(ui->operationList->currentItem()->data(OperationDelegate::IdRole).toInt());
}

void OperationsWindow::removeCurrentItem()
{
    OperationManager::get()->remove(ui->operationList->currentItem()->data(OperationDelegate::IdRole).toInt());
}
