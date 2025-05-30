#include "applicationswindow.h"

ApplicationsWindow::ApplicationsWindow(QWidget *parent, QStringList files) :
    QMainWindow(parent),
    ui(new Ui::ApplicationsWindow)
{
    ui->setupUi(this);

#ifdef MAEMO
  setProperty("X-Maemo-Orientation", 2);
  setProperty("X-Maemo-StackedWindow", 1);
#endif
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->applicationList->setItemDelegate(new DescriptiveDelegate(this));

    connect(ui->applicationList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showApplicationMenu(QPoint)));
    connect(ui->addAction, SIGNAL(triggered()), this, SLOT(onAddClicked()));
    connect(ui->clearAction, SIGNAL(triggered()), this, SLOT(onClearClicked()));

    if (files.isEmpty()) {
        // Edit mode
        connect(ui->applicationList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(editCurrentApplication()));
    } else {
        // Open mode
        this->files = files;
        this->setWindowTitle("Open with...");
        connect(ui->applicationList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    }

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(close()));

    // Create some example entries
    if (!QSettings().contains("Applications/Names")) {
        QSettings().setValue("Applications/Names",
                             QStringList()
                             << "Notes"
                             << "Terminal (current dir)");

        QSettings().setValue("Applications/Commands",
                             QStringList()
                             << "osso_notes %s"
                             << "osso-xterm \"sh -c 'cd \"\"\"$(dirname \"\"%s\"\")\"\"\"; sh'\"");
    }

    QStringList names = QSettings().value("Applications/Names").toStringList();
    QStringList commands = QSettings().value("Applications/Commands").toStringList();

    for (int i = 0; i < names.size() && i < commands.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(names.at(i));
        item->setData(DescriptiveDelegate::DescriptionRole, commands.at(i));
        ui->applicationList->addItem(item);
    }

    updateVisibility();
}

ApplicationsWindow::~ApplicationsWindow()
{
    delete ui;
}

void ApplicationsWindow::updateVisibility()
{
    if (ui->applicationList->count() == 0) {
        ui->applicationList->hide();
        ui->emptyInfo->show();
    } else {
        ui->emptyInfo->hide();
        ui->applicationList->show();
    }
}

void ApplicationsWindow::onItemActivated(QListWidgetItem *item)
{
    QString commandTemplate = item->data(DescriptiveDelegate::DescriptionRole).toString();

    for (int i = 0; i < files.size(); i++)
        files[i].replace("\"", "\"\"\"").prepend('"').append('"');

    for (int i = 0; i < files.size(); i++) {
        QString command = QString(commandTemplate).replace("%s", files.at(i));
        qDebug() << "Executing:" << command;
        QProcess::startDetached(command);
    }

    this->close();
}

void ApplicationsWindow::showApplicationMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Edit"), this, SLOT(editCurrentApplication()));
    contextMenu->addAction(tr("Remove"), this, SLOT(removeCurrentApplication()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void ApplicationsWindow::editCurrentApplication()
{
    QListWidgetItem *item = ui->applicationList->currentItem();
    ApplicationDialog applicationDialog(this, item->text(), item->data(DescriptiveDelegate::DescriptionRole).toString());

    if (applicationDialog.exec() == QDialog::Accepted) {
        item->setText(applicationDialog.nameEdit->text());
        item->setData(DescriptiveDelegate::DescriptionRole, applicationDialog.commandEdit->text());
        save();
    }
}

void ApplicationsWindow::removeCurrentApplication()
{
    delete ui->applicationList->currentItem();

    save();

    updateVisibility();
}

void ApplicationsWindow::onAddClicked()
{
    ApplicationDialog applicationDialog(this);

    if (applicationDialog.exec() == QDialog::Accepted) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(applicationDialog.nameEdit->text());
        item->setData(DescriptiveDelegate::DescriptionRole, applicationDialog.commandEdit->text());
        ui->applicationList->addItem(item);

        ui->emptyInfo->hide();
        ui->applicationList->show();

        save();

        updateVisibility();
    }
}

void ApplicationsWindow::onClearClicked()
{
    if (ConfirmDialog(this, ConfirmDialog::ClearApplications).exec() == QMessageBox::Yes) {
        ui->applicationList->clear();

        save();

        updateVisibility();
    }
}

void ApplicationsWindow::save()
{
    QStringList names;
    QStringList commands;

    for (int i = 0; i < ui->applicationList->count(); i++) {
        names.append(ui->applicationList->item(i)->text());
        commands.append(ui->applicationList->item(i)->data(DescriptiveDelegate::DescriptionRole).toString());
    }

    QSettings().setValue("Applications/Names", names);
    QSettings().setValue("Applications/Commands", commands);
}
