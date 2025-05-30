#include "mainwindow.h"

MainWindow::MainWindow(QFileInfo startingLocation) :
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef MAEMO
    setProperty("X-Maemo-Orientation", 2);
    setProperty("X-Maemo-StackedWindow", 1);
#endif

    // Set icons
    ui->locationToggleButton->setIcon(QIcon::fromTheme("keyboard_close"));
    ui->selectionToggleButton->setIcon(QIcon::fromTheme("general_add"));
    ui->bookmarksButton->setIcon(QIcon::fromTheme("general_bookmark"));
    ui->clipboardButton->setIcon(QIcon::fromTheme("email_attachment"));
    ui->fullscreenToggleButton->setIcon(QIcon::fromTheme("general_fullsize"));
    ui->searchCloseButton->setIcon(QIcon::fromTheme("general_close"));

    // Navigation
    connect(ui->locationWidget, SIGNAL(locationSelected(QString)), ui->browserList, SLOT(openPath(QString)));
    connect(ui->browserList, SIGNAL(locationChanged(QString,int)), this, SLOT(onLocationChanged(QString,int)));
    connect(ui->browserList, SIGNAL(loadingStarted()), this, SLOT(onLoadingStarted()));
    connect(ui->browserList, SIGNAL(loadingFinished()), this, SLOT(onLoadingFinished()));

    // Buttons panel
    connect(ui->locationToggleButton, SIGNAL(toggled(bool)), ui->locationWidget, SLOT(setHidden(bool)));
    connect(ui->locationToggleButton, SIGNAL(toggled(bool)), ui->locationLabel, SLOT(setVisible(bool)));
    connect(ui->locationToggleButton, SIGNAL(toggled(bool)), ui->browserList, SLOT(enableLocationItem(bool)));
    connect(ui->selectionToggleButton, SIGNAL(toggled(bool)), ui->browserList, SLOT(enableMultiSelection(bool)));
    connect(ui->bookmarksButton, SIGNAL(clicked()), this, SLOT(showBookmarks()));
    connect(ui->clipboardButton, SIGNAL(clicked()), this, SLOT(showClipboard()));
    connect(ui->fullscreenToggleButton, SIGNAL(toggled(bool)), this, SLOT(enableFullscreen(bool)));

    // Location bar visibility
    ui->locationToggleButton->setChecked(QSettings().value("Browser/HideLocationBar", DEFAULT_HideLocationBar).toBool());

    // Buttons shortcuts
    ui->selectionToggleButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    ui->bookmarksButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    ui->clipboardButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));

    // Search box
    connect(ui->searchCloseButton, SIGNAL(clicked()), this, SLOT(closeSearch()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));

    // Menu actions
    connect(ui->settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));
    connect(ui->createAction, SIGNAL(triggered()), this, SLOT(showCreateDialog()));
    connect(ui->applicationsAction, SIGNAL(triggered()), this, SLOT(showApplications()));
    connect(ui->gridAction, SIGNAL(toggled(bool)), ui->browserList, SLOT(enableGridMode(bool)));
    connect(ui->hiddenAction, SIGNAL(toggled(bool)), ui->browserList, SLOT(enableHidden(bool)));
    connect(ui->operationsAction, SIGNAL(triggered()), this, SLOT(showOperations()));
    connect(ui->selectAction, SIGNAL(triggered()), this, SLOT(onSelectClicked()));
    connect(ui->clipAction, SIGNAL(triggered()), ui->browserList, SLOT(clipSelected()));

    // Sorting buttons
    QActionGroup *sortGroup = new QActionGroup(this);
    sortGroup->setExclusive(true);
    QAction *nameSortAction = new QAction(tr("Name"), sortGroup);
    QAction *timeSortAction = new QAction(tr("Date"), sortGroup);
    QAction *sizeSortAction = new QAction(tr("Size"), sortGroup);
    QAction *typeSortAction = new QAction(tr("Type"), sortGroup);
    nameSortAction->setCheckable(true);
    timeSortAction->setCheckable(true);
    sizeSortAction->setCheckable(true);
    typeSortAction->setCheckable(true);
    ui->windowMenu->addActions(sortGroup->actions());

    ui->gridAction->setChecked(QSettings().value("Browser/GridView", DEFAULT_GridView).toBool());
    ui->hiddenAction->setChecked(QSettings().value("Browser/ShowHidden", DEFAULT_ShowHidden).toBool());

    // Menu shortcuts
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_G), this), SIGNAL(activated()), ui->gridAction, SLOT(trigger()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this), SIGNAL(activated()), ui->hiddenAction, SLOT(trigger()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this), SIGNAL(activated()), ui->operationsAction, SLOT(trigger()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_A), this), SIGNAL(activated()), ui->selectAction, SLOT(trigger()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this), SIGNAL(activated()), ui->clipAction, SLOT(trigger()));

    // Sorting actions
    connect(nameSortAction, SIGNAL(triggered()), ui->browserList, SLOT(sortByName()));
    connect(timeSortAction, SIGNAL(triggered()), ui->browserList, SLOT(sortByTime()));
    connect(sizeSortAction, SIGNAL(triggered()), ui->browserList, SLOT(sortBySize()));
    connect(typeSortAction, SIGNAL(triggered()), ui->browserList, SLOT(sortByType()));

    switch (QSettings().value("Browser/Sorting", DEFAULT_Sorting).toInt()) {
        case QDir::Name: nameSortAction->trigger(); break;
        case QDir::Time: timeSortAction->trigger(); break;
        case QDir::Size: sizeSortAction->trigger(); break;
        case QDir::Type: typeSortAction->trigger(); break;
    }

    // Other shortcuts
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this), SIGNAL(activated()), ui->locationWidget, SLOT(enableEditor()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(goBack()));
    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Backspace), this), SIGNAL(activated()), this, SLOT(goForward()));

    // Operation manager prompts
    connect(OperationManager::get(), SIGNAL(overwriteSituation(int,FileOperation::Type,QString,QString)),
            this, SLOT(showOverwriteDialog(int,FileOperation::Type,QString,QString)));
    connect(OperationManager::get(), SIGNAL(selfOverwriteSituation(int,FileOperation::Type,QString,QString)),
            this, SLOT(showSelfOverwriteDialog(int,FileOperation::Type,QString,QString)));
    connect(OperationManager::get(), SIGNAL(errorSituation(int,FileOperation::Type,QString,QString)),
            this, SLOT(showErrorDialog(int,FileOperation::Type,QString,QString)));

    ui->locationLabel->installEventFilter(this);

    ui->browserList->viewport()->installEventFilter(this);
    for(QPushButton* button: ui->toolbarWidget->findChildren<QPushButton*>())
        button->installEventFilter(this);

    // Setup rotation handling
    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(onOrientationChanged(int,int)));
    rotator->setSlave(this);

    // Should be called before setting the initial location, to avoid unnecessary file listings
    reloadSettings();

    // Check whether the standard starting location should be overriden
    const bool standardStartingLocation = !startingLocation.exists() || !startingLocation.isDir();

    // Determine location for the initial view
    if (standardStartingLocation) {
        // Custom location was invalid or not set, load from settings
        startingLocation = QSettings().value("Main/StartingLocation", DEFAULT_StartingLocation).toString();
        if (!startingLocation.exists() || !startingLocation.isDir()) {
            // Recover from invalid location in the settings
            startingLocation = QFileInfo(DEFAULT_StartingLocation);
            if (!startingLocation.exists() || !startingLocation.isDir()) {
                // Recover from invalid default location
                startingLocation = QFileInfo("/");
            }
        }
    }

    // Prepare the initial view
    ui->browserList->openPath(startingLocation.absoluteFilePath());

    // Start in bookmarks if configured to do so
    if (standardStartingLocation && QSettings().value("Main/StartInBookmarks", DEFAULT_StartInBookmarks).toBool())
        showBookmarks();
}

MainWindow::~MainWindow()
{
    QSettings().setValue("Browser/HideLocationBar", ui->locationToggleButton->isChecked());
    QSettings().setValue("Browser/GridView", ui->gridAction->isChecked());
    QSettings().setValue("Browser/ShowHidden", ui->hiddenAction->isChecked());

    if (QSettings().value("Main/SaveLocation", DEFAULT_SaveLocation).toBool())
        QSettings().setValue("Main/StartingLocation", ui->browserList->currentPath());

    delete Clipboard::get();

    delete ui;
}

bool MainWindow::eventFilter(QObject* obj, QEvent *e)
{
    if (obj == ui->locationLabel) {
        if (e->type() == QEvent::Resize)
            ui->locationLabel->setText(QFontMetrics(ui->locationLabel->font()).elidedText(ui->browserList->currentPath(), Qt::ElideLeft, ui->locationLabel->width()));
    }

    // Check for swipe-up on the list view
    else if (obj == ui->browserList->viewport() && e->type() == QEvent::MouseButtonPress) {
        if (ui->toolbarLayout->direction() == QBoxLayout::TopToBottom
        && static_cast<QMouseEvent*>(e)->y() > ui->browserList->viewport()->height() - 25)
            ui->searchWidget->show();
    }

    // Check for swipe-up on the toolbar buttons
    else if (e->type() == QEvent::MouseMove) {
        if (ui->toolbarLayout->direction() == QBoxLayout::LeftToRight
        && static_cast<QMouseEvent*>(e)->y() < -25)
            ui->searchWidget->show();
    }

    return false;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Enter:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Backspace:
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->browserList->setFocus();
            break;

        case Qt::Key_AltGr:
            if (ui->locationWidget->isEditorEnabled())
                break;

        default:
            if (!ui->searchEdit->hasFocus()) {
                ui->searchWidget->show();
                ui->searchEdit->setText(ui->searchEdit->text() + e->text());
                ui->searchEdit->setFocus();
            }
            break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->browserList->setFocus();
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    // TODO: Maybe also add a note about queued operations
    if (!OperationManager::get()->activeOperations.isEmpty()) {
        if (ConfirmDialog(this, ConfirmDialog::ForceAbort).exec() != QMessageBox::Yes) {
            e->ignore();
            return;
        }
    }

    QMainWindow::closeEvent(e);
}

void MainWindow::reloadSettings()
{
    QString orientation = QSettings().value("Main/Orientation", DEFAULT_Orientation).toString();
    Rotator::acquire()->setPolicy(orientation == "landscape" ? Rotator::Landscape :
                                  orientation == "portrait"  ? Rotator::Portrait  :
                                                               Rotator::Automatic);

    ui->browserList->enableThumbnails(QSettings().value("Browser/ShowThumbnails", DEFAULT_ShowThumbnails).toBool());

    OperationManager::get()->setArchiveSize(QSettings().value("Operations/ArchiveSize", DEFAULT_ArchiveSize).toInt());

    OperationManager::get()->setActiveLimit(QSettings().value("Operations/ActiveLimiter", DEFAULT_ActiveLimiter).toBool() ?
                                            QSettings().value("Operations/MaxActive", DEFAULT_MaxActive).toInt() : -1);

    ui->browserList->setDetails(static_cast<FileSystemModel::Detail>(QSettings().value("Browser/FirstDetail", DEFAULT_FirstDetail).toInt()),
                                static_cast<FileSystemModel::Detail>(QSettings().value("Browser/SecondDetail", DEFAULT_SecondDetail).toInt()));

    ui->locationWidget->reloadSettings();
}

QMainWindow* MainWindow::findTopWindow()
{
    // This will only work for simple cases, but only those are currently possible
    QList<QMainWindow*> topWindow = findChildren<QMainWindow*>();
    return topWindow.isEmpty() ? this : topWindow.first();
}

void MainWindow::onSearchTextChanged(QString text)
{
    qobject_cast<QSortFilterProxyModel*>(ui->browserList->model())->setFilterFixedString(text);

    if (text.isEmpty()) {
        ui->searchWidget->hide();
    }
}

void MainWindow::closeSearch()
{
    if (ui->searchEdit->text().isEmpty())
        ui->searchWidget->hide();
    else
        ui->searchEdit->clear();
}

// Update the displayed path
void MainWindow::onLocationChanged(QString path, int oldPosition)
{
    history.add(path);

    closeSearch();
    ui->selectionToggleButton->setChecked(false);
    ui->browserList->setResumePosition(ui->locationWidget->setLocation(path, oldPosition));
    ui->locationLabel->setText(QFontMetrics(ui->locationLabel->font()).elidedText(path, Qt::ElideLeft, ui->locationLabel->width()));
}

void MainWindow::onLoadingStarted()
{
#ifdef MAEMO
    setProperty("X-Maemo-Progress", true);
#endif
}

void MainWindow::onLoadingFinished()
{
#ifdef MAEMO
    setProperty("X-Maemo-Progress", false);
#endif
}

// Adapt the layout to the screen oriention
void MainWindow::onOrientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->mainLayout->removeWidget(ui->toolbarWidget);
        ui->toolbarLayout->removeItem(ui->toolbarSpacer);
        ui->toolbarLayout->insertSpacerItem(1, ui->toolbarSpacer);
        ui->toolbarLayout->setDirection(QBoxLayout::TopToBottom);
        ui->mainLayout->addWidget(ui->toolbarWidget, 0, 1, 6, 1);
    } else { // Portrait
        ui->mainLayout->removeWidget(ui->toolbarWidget);
        ui->toolbarLayout->removeItem(ui->toolbarSpacer);
        ui->toolbarLayout->setDirection(QBoxLayout::LeftToRight);
        ui->mainLayout->addWidget(ui->toolbarWidget, 3, 0, 1, 1);
    }
}

// Set fullscreen mode on or off
void MainWindow::enableFullscreen(bool enable)
{
    if (enable) {
        this->showFullScreen();
    } else {
        this->showNormal();
    }
}

// Open the settings window and update settings when finished
void MainWindow::showSettings()
{
    SettingsWindow *settings = new SettingsWindow(ui->browserList->currentPath(), this);
    connect(settings, SIGNAL(destroyed()), this, SLOT(reloadSettings()));
    settings->show();
}

// Open the operations window
void MainWindow::showOperations()
{
    OperationsWindow *operations = new OperationsWindow(this);
    operations->show();
}

void MainWindow::showApplications()
{
    ApplicationsWindow *applications = new ApplicationsWindow(this);
    applications->show();
}

// Open the clipboard window
void MainWindow::showClipboard()
{
    ClipboardWindow *clipboard = new ClipboardWindow(ui->browserList->currentPath(), this);
    clipboard->show();
}

// Open the settings window and update settings when finished
void MainWindow::showBookmarks()
{
    BookmarksWindow *bookmarks = new BookmarksWindow(ui->browserList->currentPath(), this);
    connect(bookmarks, SIGNAL(locationSelected(QString)), ui->browserList, SLOT(openPath(QString)));
    bookmarks->show();
}

void MainWindow::showCreateDialog()
{
    CreateDialog(ui->browserList->currentPath(), this).exec();
}

void MainWindow::showOverwriteDialog(int id, FileOperation::Type type, QString source, QString target)
{
    AttentionDialog(AttentionDialog::Overwrite,
                    id, type, source, target,
                    findTopWindow()).exec();
}

void MainWindow::showSelfOverwriteDialog(int id, FileOperation::Type type, QString source, QString target)
{
    AttentionDialog(AttentionDialog::SelfOverwrite,
                    id, type, source, target,
                    findTopWindow()).exec();
}

void MainWindow::showErrorDialog(int id, FileOperation::Type type, QString source, QString target)
{
    AttentionDialog(AttentionDialog::Error,
                    id, type, source, target,
                    findTopWindow()).exec();
}

// Select all items in the current directory
void MainWindow::onSelectClicked()
{
    ui->selectionToggleButton->setChecked(true);
    ui->browserList->selectAll();
}

void MainWindow::goBack()
{
    history.setPosition(ui->browserList->position());

    HistoryItem location = history.back();

    if (!location.path.isNull()) {
        ui->browserList->openPath(location.path);
        ui->browserList->setResumePosition(location.position);
    }
}

void MainWindow::goForward()
{
    history.setPosition(ui->browserList->position());

    HistoryItem location = history.forward();

    if (!location.path.isNull()) {
        ui->browserList->openPath(location.path);
        ui->browserList->setResumePosition(location.position);
    }
}
