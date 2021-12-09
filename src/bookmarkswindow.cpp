#include "bookmarkswindow.h"

BookmarksWindow::BookmarksWindow(QString currentPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BookmarksWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->currentPath = currentPath;

    ui->bookmarkList->setItemDelegate(new DescriptiveDelegate(this));

    connect(ui->bookmarkList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->bookmarkList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showBookmarkMenu(QPoint)));
    connect(ui->bookmarkList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(save()));

    connect(ui->addAction, SIGNAL(triggered()), this, SLOT(onAddClicked()));
    connect(ui->clearAction, SIGNAL(triggered()), this, SLOT(onClearClicked()));

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(close()));

    QStringList names = QSettings().value("Bookmarks/Names").toStringList();
    QStringList paths = QSettings().value("Bookmarks/Paths").toStringList();

    for (int i = 0; i < names.size() && i < paths.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();

        item->setText(names.at(i));
        item->setData(DescriptiveDelegate::DescriptionRole, paths.at(i));
        item->setIcon(FileSystemModel::icon(paths.at(i)));
        item->setFlags(item->flags () | Qt::ItemIsEditable);

        ui->bookmarkList->addItem(item);
    }

    updateVisibility();
}

BookmarksWindow::~BookmarksWindow()
{
    delete ui;
}

void BookmarksWindow::updateVisibility()
{
    if (ui->bookmarkList->count() == 0) {
        ui->bookmarkList->hide();
        ui->emptyInfo->show();
    } else {
        ui->emptyInfo->hide();
        ui->bookmarkList->show();
    }
}

void BookmarksWindow::onItemActivated(QListWidgetItem *item)
{
    emit locationSelected(item->data(DescriptiveDelegate::DescriptionRole).toString());

    this->close();
}

void BookmarksWindow::showBookmarkMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Rename"), this, SLOT(renameCurrentBookmark()));
    contextMenu->addAction(tr("Remove"), this, SLOT(removeCurrentBookmark()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void BookmarksWindow::renameCurrentBookmark()
{
    ui->bookmarkList->editItem(ui->bookmarkList->currentItem());
}

void BookmarksWindow::removeCurrentBookmark()
{
    delete ui->bookmarkList->currentItem();

    save();

    updateVisibility();
}

void BookmarksWindow::onAddClicked()
{
    QListWidgetItem *item = new QListWidgetItem();

    item->setText(currentPath.mid(currentPath.lastIndexOf('/')+1));
    item->setData(DescriptiveDelegate::DescriptionRole, currentPath);
    item->setIcon(FileSystemModel::icon(currentPath));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    ui->bookmarkList->addItem(item);

    save();

    updateVisibility();
}

void BookmarksWindow::onClearClicked()
{
    if (ConfirmDialog(this, ConfirmDialog::ClearBookmarks).exec() == QMessageBox::Yes) {
        ui->bookmarkList->clear();

        save();

        updateVisibility();
    }
}

void BookmarksWindow::save()
{
    QStringList names;
    QStringList paths;

    for (int i = 0; i < ui->bookmarkList->count(); i++) {
        names.append(ui->bookmarkList->item(i)->text());
        paths.append(ui->bookmarkList->item(i)->data(DescriptiveDelegate::DescriptionRole).toString());
    }

    QSettings().setValue("Bookmarks/Names", names);
    QSettings().setValue("Bookmarks/Paths", paths);
}
