#include "locationwidget.h"

#define BUTTON_HEIGHT 60
#define EDITOR_HEIGHT 70

LocationWidget::LocationWidget(QWidget *parent) :
    QScrollArea(parent)
{
    // Button widget
    buttonWidget = new QWidget();
    buttonWidget->setFixedHeight(BUTTON_HEIGHT);
    buttonWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    // Button layout
    QLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(5, 0, 40, 0);
    buttonLayout->setSpacing(0);

    checkedButton = NULL;

    // Editor widget
    editorWidget = new QWidget();
    editorWidget->setFixedHeight(EDITOR_HEIGHT);

    // Editor layout
    QLayout *editorLayout = new QHBoxLayout(editorWidget);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    // Editor field
    locationEdit = new QLineEdit(editorWidget);
    connect(locationEdit, SIGNAL(returnPressed()), this, SLOT(emitLocationFromEditor()));

    // Path completion
    QCompleter *completer = new QCompleter(locationEdit);
    QFileSystemModel *completerModel = new QFileSystemModel(locationEdit);
    completerModel->setFilter(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    completer->setModel(completerModel);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    locationEdit->setCompleter(completer);

    // Editor buttons
    locationUpButton = new QToolButton(editorWidget);
    QToolButton *locationAcceptButton = new QToolButton(editorWidget);
    QToolButton *locationCancelButton = new QToolButton(editorWidget);
    locationUpButton->setIcon(QIcon::fromTheme("filemanager_folder_up"));
    locationAcceptButton->setIcon(QIcon::fromTheme("widgets_tickmark_list"));
    locationCancelButton->setIcon(QIcon::fromTheme("general_close"));
    locationUpButton->setFixedWidth(64);
    locationAcceptButton->setFixedWidth(48);
    locationCancelButton->setFixedWidth(48);
    connect(locationUpButton, SIGNAL(clicked()), this, SLOT(goUp()));
    connect(locationCancelButton, SIGNAL(clicked()), this, SLOT(disableEditor()));
    connect(locationAcceptButton, SIGNAL(clicked()), this, SLOT(emitLocationFromEditor()));

    // Add the prepared components to the editor widget
    editorLayout->addWidget(locationUpButton);
    editorLayout->addWidget(locationEdit);
    editorLayout->addWidget(locationAcceptButton);
    editorLayout->addWidget(locationCancelButton);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setWidgetResizable(true);

    // Choose the initial view
    if (isEditorPersistent()
    &&  QSettings().value("Browser/ShowEditor").toBool())
    {
        enableEditor();
    } else {
        disableEditor();
    }

    // Settings should be reloaded by the owner of this widget
}

LocationWidget::~LocationWidget()
{
    QSettings().setValue("Browser/ShowEditor", isEditorEnabled());

    this->takeWidget(); // ???
    delete buttonWidget;
    delete editorWidget;
}

void LocationWidget::reloadSettings()
{
    locationUpButton->setVisible(isEditorPersistent());
}

bool LocationWidget::isEditorPersistent()
{
    return QSettings().value("Browser/PersistentEditor", DEFAULT_PersistentEditor).toBool();
}

bool LocationWidget::isEditorEnabled()
{
    return this->widget() == editorWidget;
}

void LocationWidget::enableEditor()
{
    if (this->widget() != editorWidget) {
        this->takeWidget();
        this->setFixedHeight(EDITOR_HEIGHT);
        this->setWidget(editorWidget);
        locationEdit->setText(currentPath);
        qobject_cast<QFileSystemModel*>(locationEdit->completer()->model())->setRootPath(currentPath);
    }

    locationEdit->setFocus();
}

void LocationWidget::disableEditor()
{
    if (this->widget() != buttonWidget) {
        this->takeWidget();
        this->setFixedHeight(BUTTON_HEIGHT);
        this->setWidget(buttonWidget);
    }

    buttonWidget->adjustSize();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    centerCheckedButton();
}

void LocationWidget::emitLocationFromEditor()
{
    if (!isEditorPersistent())
        disableEditor();

    if (currentPath != locationEdit->text())
        emit locationSelected(locationEdit->text());
}

int LocationWidget::setLocation(QString path, int oldPosition)
{
    currentPath = path;

    int newPosition = 0;

    QLayout *buttonLayout = buttonWidget->layout();
    QStringList pathParts = path.split('/', QString::SkipEmptyParts);
    pathParts.prepend("/");
    int i;

    // Try to reuse some of the old buttons
    for (i = 0; i < buttonLayout->count(); i++) {
        // This layout should contain only buttons
        QAbstractButton *button = qobject_cast<QAbstractButton*>(buttonLayout->itemAt(i)->widget());

        if (i < pathParts.count()) {
            // Examine the button to see what modifications are necessary
            if (button->text() == pathParts.at(i)) {
                // The new path is a substring of the old path so far
                if (i == pathParts.size()-1) {
                    // Finished without mismatches
                    if (button == checkedButton) {
                        newPosition = oldPosition;
                    } else {
                        button->setCheckable(true);
                        button->setChecked(true);
                        checkedButton = button;
                        newPosition = scrollPositions.at(i);
                    }
                } else {
                    // Make sure that no unnecessary buttons on the way are highlighted
                    if (button->isChecked()) {
                        button->setChecked(false);
                        button->setCheckable(false);
                        scrollPositions[i] = oldPosition;
                    }
                }
            } else {
                // Remove all mismatching buttons
                while (buttonLayout->itemAt(i)) {
                    delete buttonLayout->takeAt(i)->widget();
                    scrollPositions.removeAt(i);
                }
                break;
            }
        } else {
            // The path is already parsed, so only uncheck the remaining buttons
            if (button->isChecked()) {
                button->setChecked(false);
                button->setCheckable(false);
                scrollPositions[i] = oldPosition;
            }
        }
    }

    // Add more buttons if necessary
    if (i < pathParts.size()) {
        // Remaining buttons for the new path, except the last one
        while(i < pathParts.size()-1) {
            buttonLayout->addWidget(createButton(pathParts.at(i++)));
            scrollPositions.append(0);
        }

        // Add and highlight the last button
        QAbstractButton *lastButton = createButton(pathParts.last());
        lastButton->setCheckable(true);
        lastButton->setChecked(true);
        checkedButton = lastButton;
        buttonLayout->addWidget(lastButton);
        scrollPositions.append(0);
    }

    if (isEditorPersistent()) {
        locationEdit->setText(currentPath);
    } else {
        disableEditor();
    }

    locationUpButton->setEnabled(currentPath != "/");

    return newPosition;
}

void LocationWidget::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    centerCheckedButton();
}

void LocationWidget::centerCheckedButton()
{
    if (checkedButton)
        this
        ->property("kineticScroller").value<QAbstractKineticScroller*>()
        ->scrollTo(QPoint(qBound(0,
                                 checkedButton->x() + (checkedButton->width() - this->viewport()->width()) / 2,
                                 this->horizontalScrollBar()->maximum()),
                          0));
}

QAbstractButton* LocationWidget::createButton(QString name)
{
    QPushButton *button = new QPushButton(name);
    button->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    connect(button, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(enableEditor()));
    return button;
}

void LocationWidget::onButtonClicked()
{
    QString path;
    QLayout *buttonLayout = buttonWidget->layout();

    for (int i = 0; i < buttonLayout->count(); i++) {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(buttonLayout->itemAt(i)->widget());
        if (button) {
            path.append(button->text());
            if (button == this->sender()) {
                button->setChecked(true);
                break;
            }
            path.append('/');
        }
    }

    emit locationSelected(path);
}

void LocationWidget::goUp()
{
    emit locationSelected("..");
}
