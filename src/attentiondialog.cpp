#include "attentiondialog.h"

AttentionDialog::AttentionDialog(Type type, int id, FileOperation::Type opType, QString source, QString target, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttentionDialog)
{
    ui->setupUi(this);

    this->operationId = id;
    this->target = target;

    // Set color for the lables
    QPalette labelPalette = ui->sourceLabel->palette();
    labelPalette.setBrush(QPalette::Active, QPalette::WindowText, QBrush(QMaemo5Style::standardColor("SecondaryTextColor")));
    ui->sourceLabel->setPalette(labelPalette);
    ui->targetLabel->setPalette(labelPalette);

    // Set font for the labels
    QFont labelFont = ui->sourceLabel->font();
    labelFont.setPointSize(13);
    ui->sourceLabel->setFont(labelFont);
    ui->targetLabel->setFont(labelFont);

    // Fill in the source line
    ui->sourceLabel->setText(FileOperation::labelForType(opType));
    ui->sourceInfo->setText(source);

    // Fill in the target line
    if (opType == FileOperation::Remove) {
        ui->targetLabel->hide();
        ui->targetInfo->hide();
    } else {
        ui->targetLabel->setText("To");
        ui->targetInfo->setText(target);
    }

    // Adjust type-specific features
    if (type == Error) {
        this->setWindowTitle(tr("Error"));

        ui->targetInfo->setReadOnly(true);

        ui->acceptButton->hide();
        ui->renameButton->hide();

        connect(ui->retryButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    } else {
        if (type == Overwrite) {
            this->setWindowTitle(tr("Overwrite warning"));

            ui->renameButton->hide();

            connect(ui->acceptButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
        }
        else { // type == SelfOverwrite
            this->setWindowTitle(tr("Self-overwrite warning"));

            ui->acceptButton->hide();

            connect(ui->renameButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
        }

        ui->retryButton->hide();

        ui->targetInfo->setFocus();
        ui->targetInfo->setValidator(new PrefixValidator(target.left(target.lastIndexOf("/")+1), ui->targetInfo));
        connect(ui->targetInfo, SIGNAL(textChanged(QString)), this, SLOT(onOverwriteTargetChanged(QString)));

        // Select only the base name
        int slashIndex = ui->targetInfo->text().lastIndexOf("/")+1;
        int dotIndex   = ui->targetInfo->text().lastIndexOf(".");
        if (dotIndex <= slashIndex) dotIndex = ui->targetInfo->text().length();
        ui->targetInfo->setSelection(slashIndex, dotIndex-slashIndex);
    }

    connect(ui->skipButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    connect(ui->abortButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
}

AttentionDialog::~AttentionDialog()
{
    // If user did not press a button, pause the operation and ask again when resumed
    if (operationId != -1)
        OperationManager::get()->setAttentionAction(operationId, FileOperation::Ask);

    delete ui;
}

void AttentionDialog::onOverwriteTargetChanged(QString target)
{
    int dummyPos;
    ui->acceptButton->setEnabled(ui->targetInfo->validator()->validate(target, dummyPos) == QValidator::Acceptable);
}

void AttentionDialog::onSelfOverwriteTargetChanged(QString target)
{
    int dummyPos;
    ui->renameButton->setEnabled(ui->targetInfo->validator()->validate(target, dummyPos) == QValidator::Acceptable &&
                                 ui->targetInfo->text() != this->target);
}

void AttentionDialog::onButtonClicked()
{
    FileOperation::Action action;
    QObject *button = this->sender();

    // Decide which action should be taken, based on the pressed button and state of the checkbox
    if (button == ui->acceptButton)
        action = ui->allBox->isChecked() ? FileOperation::AllowAll : FileOperation::Allow;
    else if (button == ui->skipButton)
        action = ui->allBox->isChecked() ? FileOperation::SkipAll : FileOperation::Skip;
    else if (button == ui->renameButton)
        action = FileOperation::Allow;
    else if (button == ui->retryButton)
        action = FileOperation::Retry;
    else // button == ui->abortButton
        action = FileOperation::Abort;

    // Send user's response to the appropriate operation
    OperationManager::get()->setAttentionAction(operationId, action,
                                                ui->targetInfo->text() == target ? QString() : ui->targetInfo->text());

    // Mark the operation as handled and destroy the window
    operationId = -1;
    this->close();
}
