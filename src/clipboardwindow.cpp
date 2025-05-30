#include "clipboardwindow.h"

ClipboardWindow::ClipboardWindow(QString currentPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClipboardWindow)
{
    ui->setupUi(this);


#ifdef MAEMO
    setProperty("X-Maemo-Orientation", 2);
    setProperty("X-Maemo-StackedWindow", 1);
#endif
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->currentPath = currentPath;

    ui->fileList->setItemDelegate(new DescriptiveDelegate(this));

    // Clipboard management
    connect(ui->selectAction, SIGNAL(triggered()), ui->fileList, SLOT(selectAll()));
    connect(ui->deselectAction, SIGNAL(triggered()), ui->fileList, SLOT(clearSelection()));
    connect(ui->unclipAction, SIGNAL(triggered()), this, SLOT(onUnclipClicked()));
    connect(ui->clearAction, SIGNAL(triggered()), this, SLOT(onClearClicked()));

    // File operations
    connect(ui->copyAction, SIGNAL(triggered()), this, SLOT(onCopyClicked()));
    connect(ui->moveAction, SIGNAL(triggered()), this, SLOT(onMoveClicked()));
    connect(ui->linkAction, SIGNAL(triggered()), this, SLOT(onLinkClicked()));
    connect(ui->removeAction, SIGNAL(triggered()), this, SLOT(onRemoveClicked()));

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(close()));

    listFiles();
}

ClipboardWindow::~ClipboardWindow()
{
    syncSelection();

    delete ui;
}

// Apply local selection changes and count selected items
int ClipboardWindow::syncSelection()
{
    Clipboard *clipboard = Clipboard::get();
    int selectedCount = 0;

    for (int i = 0; i < ui->fileList->count(); i++) {
        if (ui->fileList->item(i)->isSelected()) {
            clipboard->select(i, true);
            selectedCount++;
        } else {
            clipboard->select(i, false);
        }
    }

    return selectedCount;
}

// Ceompletely refresh the list of items
void ClipboardWindow::listFiles()
{
    ui->fileList->clear();

    QList<QString> contents = Clipboard::get()->contents();
    QList<bool> selection = Clipboard::get()->selection();

    for (int i = 0; i < contents.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();

        QFileInfo info(contents.at(i));
        item->setText(info.fileName());
        item->setIcon(FileSystemModel::icon(contents.at(i)));
        item->setData(DescriptiveDelegate::DescriptionRole, info.absolutePath());

        ui->fileList->addItem(item);
        item->setSelected(selection.at(i));
    }

    if (ui->fileList->count() == 0) {
        ui->fileList->hide();
        ui->emptyInfo->show();
    } else {
        ui->emptyInfo->hide();
        ui->fileList->show();
    }
}

// Remove items which are not selected
void ClipboardWindow::onUnclipClicked()
{
    for (int i = 0; i < ui->fileList->count(); i++) {
        if (!ui->fileList->item(i)->isSelected()) {
            delete ui->fileList->item(i);
            Clipboard::get()->remove(i);
            i--;
        }
    }

    if (ui->fileList->count() == 0) {
        ui->fileList->hide();
        ui->emptyInfo->show();
    }
}

// Clear the clipboard
void ClipboardWindow::onClearClicked()
{
    Clipboard::get()->clear();
    this->close();
}

// Copy clipped files to the current diractory
void ClipboardWindow::onCopyClicked()
{
    if (syncSelection() == 0) {
        QMaemo5InformationBox::information(this, tr("Selection is empty"));
    } else {
        OperationManager::get()->add(new FileOperation(FileOperation::Copy, Clipboard::get()->selectedFiles(), currentPath));

        listFiles();
    }
}

// Move clipped files to the current directory
void ClipboardWindow::onMoveClicked()
{
    if (syncSelection() == 0) {
        QMaemo5InformationBox::information(this, tr("Selection is empty"));
    } else {
        OperationManager::get()->add(new FileOperation(FileOperation::Move, Clipboard::get()->selectedFiles(true), currentPath));

        listFiles();
    }
}

// Link clipped files to the current directory
void ClipboardWindow::onLinkClicked()
{
    if (syncSelection() == 0) {
        QMaemo5InformationBox::information(this, tr("Selection is empty"));
    } else {
        OperationManager::get()->add(new FileOperation(FileOperation::Link, Clipboard::get()->selectedFiles(), currentPath));

        listFiles();
    }
}

void ClipboardWindow::onRemoveClicked()
{
    if (syncSelection() == 0) {
        QMaemo5InformationBox::information(this, tr("Selection is empty"));
    } else {
        if (ConfirmDialog(this, ConfirmDialog::DeleteSelected).exec() == QMessageBox::Yes)
            OperationManager::get()->add(new FileOperation(FileOperation::Remove, Clipboard::get()->selectedFiles(true)));

        listFiles();
    }
}
