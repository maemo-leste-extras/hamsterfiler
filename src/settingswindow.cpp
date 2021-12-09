#include "settingswindow.h"

SettingsWindow::SettingsWindow(QString currentPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->currentPath = currentPath;
    connect(ui->startingLocationButton, SIGNAL(clicked()), this, SLOT(setStartingLocation()));

    connect(ui->activeLimiterBox, SIGNAL(toggled(bool)), ui->maxActiveBox, SLOT(setEnabled(bool)));

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(close()));

    load();
}

SettingsWindow::~SettingsWindow()
{
    save();

    delete ui;
}

void SettingsWindow::load()
{
    ui->startingLocationButton->setValueText(QSettings().value("Main/StartingLocation", DEFAULT_StartingLocation).toString());

    ui->saveLocationBox->setChecked(QSettings().value("Main/SaveLocation", DEFAULT_SaveLocation).toBool());
    ui->startInBookmarksBox->setChecked(QSettings().value("Main/StartInBookmarks", DEFAULT_StartInBookmarks).toBool());
    ui->runAsRootBox->setChecked(QSettings().value("Main/RunAsRoot", DEFAULT_RunAsRoot).toBool());

    ui->showThumbnailsBox->setChecked(QSettings().value("Browser/ShowThumbnails", DEFAULT_ShowThumbnails).toBool());
    ui->persistentEditorBox->setChecked(QSettings().value("Browser/PersistentEditor", DEFAULT_PersistentEditor).toBool());

    ui->selectNewItemsBox->setChecked(QSettings().value("Clipboard/SelectNewItems", DEFAULT_SelectNewItems).toBool());
    ui->addingClearsSelectionBox->setChecked(QSettings().value("Clipboard/AddingClearsSelection", DEFAULT_AddingClearsSelection).toBool());
    ui->queryingClearsSelectionBox->setChecked(QSettings().value("Clipboard/QueryingClearsSelection", DEFAULT_QueryingClearsSelection).toBool());
    ui->saveOnExitBox->setChecked(QSettings().value("Clipboard/SaveOnExit", DEFAULT_SaveOnExit).toBool());

    ui->archiveSizeBox->setValue(QSettings().value("Operations/ArchiveSize", DEFAULT_ArchiveSize).toInt());

    ui->activeLimiterBox->setChecked(QSettings().value("Operations/ActiveLimiter", DEFAULT_ActiveLimiter).toBool());
    ui->maxActiveBox->setValue(QSettings().value("Operations/MaxActive", DEFAULT_MaxActive).toInt());
    ui->maxActiveBox->setEnabled(ui->activeLimiterBox->isChecked());

    QMaemo5ListPickSelector *selector;
    QStandardItemModel *model;
    QString value;

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Automatic")));
    model->appendRow(new QStandardItem(tr("Landscape")));
    model->appendRow(new QStandardItem(tr("Portrait")));
    selector->setModel(model);

    value = QSettings().value("Main/Orientation", DEFAULT_Orientation).toString();
    selector->setCurrentIndex(value == "automatic" ? 0 :
                              value == "landscape" ? 1 :
                              value == "portrait"  ? 2 : 0);
    ui->orientationButton->setPickSelector(selector);

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Nothing")));
    model->appendRow(new QStandardItem(tr("Size")));
    model->appendRow(new QStandardItem(tr("Modification date")));
    model->appendRow(new QStandardItem(tr("Permissions")));
    selector->setModel(model);

    selector->setCurrentIndex(QSettings().value("Browser/FirstDetail", DEFAULT_FirstDetail).toInt());
    ui->firstDetailButton->setPickSelector(selector);

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Nothing")));
    model->appendRow(new QStandardItem(tr("Size")));
    model->appendRow(new QStandardItem(tr("Modification date")));
    model->appendRow(new QStandardItem(tr("Permissions")));
    selector->setModel(model);

    selector->setCurrentIndex(QSettings().value("Browser/SecondDetail", DEFAULT_SecondDetail).toInt());
    ui->secondDetailButton->setPickSelector(selector);
}

void SettingsWindow::save()
{
    QSettings().setValue("Main/StartingLocation", ui->startingLocationButton->valueText());

    QSettings().setValue("Main/SaveLocation", ui->saveLocationBox->isChecked());
    QSettings().setValue("Main/StartInBookmarks", ui->startInBookmarksBox->isChecked());
    QSettings().setValue("Main/RunAsRoot", ui->runAsRootBox->isChecked());

    QSettings().setValue("Browser/ShowThumbnails", ui->showThumbnailsBox->isChecked());
    QSettings().setValue("Browser/PersistentEditor", ui->persistentEditorBox->isChecked());

    QSettings().setValue("Clipboard/SelectNewItems", ui->selectNewItemsBox->isChecked());
    QSettings().setValue("Clipboard/AddingClearsSelection", ui->addingClearsSelectionBox->isChecked());
    QSettings().setValue("Clipboard/QueryingClearsSelection", ui->queryingClearsSelectionBox->isChecked());
    QSettings().setValue("Clipboard/SaveOnExit", ui->saveOnExitBox->isChecked());

    QSettings().setValue("Operations/ArchiveSize", ui->archiveSizeBox->value());

    QSettings().setValue("Operations/ActiveLimiter", ui->activeLimiterBox->isChecked());
    QSettings().setValue("Operations/MaxActive", ui->maxActiveBox->value());

    switch (static_cast<QMaemo5ListPickSelector*>(ui->orientationButton->pickSelector())->currentIndex()) {
        case 0: QSettings().setValue("Main/Orientation", "automatic"); break;
        case 1: QSettings().setValue("Main/Orientation", "landscape"); break;
        case 2: QSettings().setValue("Main/Orientation", "portrait"); break;
    }

    QSettings().setValue("Browser/FirstDetail", static_cast<QMaemo5ListPickSelector*>(ui->firstDetailButton->pickSelector())->currentIndex());
    QSettings().setValue("Browser/SecondDetail", static_cast<QMaemo5ListPickSelector*>(ui->secondDetailButton->pickSelector())->currentIndex());
}

void SettingsWindow::setStartingLocation()
{
    ui->startingLocationButton->setValueText(currentPath.isNull() ? DEFAULT_StartingLocation : currentPath);
}
