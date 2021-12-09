#include "filedetailsdialog.h"

FileDetailsDialog::FileDetailsDialog(QWidget *parent, QFileInfo info) :
    QDialog(parent),
    ui(new Ui::FileDetailsDialog)
{
    ui->setupUi(this);

    this->info = info;

    // Change color of descriptive labels
    QPalette labelPalette = ui->nameLabel->palette();
    labelPalette.setBrush(QPalette::Active, QPalette::WindowText, QBrush(QMaemo5Style::standardColor("SecondaryTextColor")));
    ui->nameLabel->setPalette(labelPalette);
    ui->targetLabel->setPalette(labelPalette);
    ui->sizeLabel->setPalette(labelPalette);
    ui->modificationLabel->setPalette(labelPalette);
    ui->ownerLabel->setPalette(labelPalette);
    ui->permissionsLabel->setPalette(labelPalette);

    // Name
    ui->nameEdit->setText(info.fileName());
    ui->nameEdit->setValidator(new QRegExpValidator(QRegExp("[^/]+"), this));

    // Symlink target
    if (info.isSymLink()) {
        char buffer[4096];
        QString linkTarget = QString::fromUtf8(buffer, readlink(info.absoluteFilePath().toUtf8(), buffer, 4096));
        ui->targetInfo->setText(linkTarget);
    } else {
        ui->targetLabel->hide();
        ui->targetInfo->hide();
    }

    // Size
    onSizeUpdated(info.size());
    if (info.isDir()) {
        sizeCounter = new SizeCounter(info);
        connect(sizeCounter, SIGNAL(finished()), sizeCounter, SLOT(deleteLater()));
        connect(sizeCounter, SIGNAL(finished()), this, SLOT(onSizeReady()));
        connect(sizeCounter, SIGNAL(sizeUpdated(qint64,int,int)), this, SLOT(onSizeUpdated(qint64,int,int)));

        // Setting the progress indicator does not work from the constructor,
        // so it has to be delayed.
        QTimer::singleShot(0, this, SLOT(onSizeStarted()));

        sizeCounter->start();
    } else {
        sizeCounter = NULL;
    }

    // Ownership
    ui->ownerInfo->setText(QString("%1:%2").arg(info.owner(), info.group()));

    // Modification date
    ui->modificationInfo->setText(FileSystemModel::dateString(info.lastModified()));

    // Permissions
    QFile::Permissions permissions = info.permissions();
    if (permissions & QFile::ReadOwner) ui->permUserReadBox->setChecked(true);
    if (permissions & QFile::WriteOwner) ui->permUserWriteBox->setChecked(true);
    if (permissions & QFile::ExeOwner) ui->permUserExeBox->setChecked(true);
    if (permissions & QFile::ReadGroup) ui->permGroupReadBox->setChecked(true);
    if (permissions & QFile::WriteGroup) ui->permGroupWriteBox->setChecked(true);
    if (permissions & QFile::ExeGroup) ui->permGroupExeBox->setChecked(true);
    if (permissions & QFile::ReadOther) ui->permOtherReadBox->setChecked(true);
    if (permissions & QFile::WriteOther) ui->permOtherWriteBox->setChecked(true);
    if (permissions & QFile::ExeOther) ui->permOtherExeBox->setChecked(true);

    // Allow renaming only if we have write permissions for the directory
    if (!(QFileInfo(info.absolutePath()).permissions() & QFile::WriteUser))
        ui->nameEdit->setEnabled(false);

    // Disable the permissions table if the file does not belong to us
    QString user = getenv("USER");
    if (!(info.owner() == user || user == "root"))
        foreach (QCheckBox *box, ui->permissionsWidget->findChildren<QCheckBox*>())
            box->setEnabled(false);

    ui->nameEdit->setFocus();

    // If the name has an extansion, select only the base
    int dotIndex = ui->nameEdit->text().lastIndexOf(".");
    if (dotIndex > 0)
        ui->nameEdit->setSelection(0, dotIndex);

    this->setMinimumHeight(800);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onOrientationChanged()));

    onOrientationChanged();
}

FileDetailsDialog::~FileDetailsDialog()
{
    if (sizeCounter)
        sizeCounter->abort();

    delete ui;
}

void FileDetailsDialog::onOrientationChanged()
{
    QRect screen = QApplication::desktop()->screenGeometry();

    ui->dialogLayout->removeWidget(ui->buttonBox);
    if (screen.width() < screen.height()) {
        // Portrait
        ui->dialogLayout->addWidget(ui->buttonBox, 1, 0);
        ui->buttonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    } else {
        // Landscape
        ui->dialogLayout->addWidget(ui->buttonBox, 0, 1);
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    }
}

void FileDetailsDialog::onSizeStarted()
{
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
}

void FileDetailsDialog::onSizeUpdated(qint64 size, int dirs, int files)
{
    QString text = FileSystemModel::sizeString(size)
                 + QString(" (%L1 B)").arg(size);

    if (dirs != -1)
        text += QString("\n%1, %2")
                .arg(tr("%Ln file(s)", "", files))
                .arg(tr("%Ln subdir(s)", "", dirs));

    ui->sizeInfo->setText(text);
}

void FileDetailsDialog::onSizeReady()
{
    sizeCounter = NULL;

    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
}

void FileDetailsDialog::save()
{

    // Abort if the filename is empty
    if (ui->nameEdit->text().isEmpty()) {
        QMaemo5InformationBox::information(this, tr("Filename is empty"));
        return;
    }

    QFile file(info.absoluteFilePath());

    // Save the permissions only if they were enabled
    if (ui->permUserReadBox->isEnabled()) {
        QFile::Permissions permissions;
        if (ui->permUserReadBox->isChecked()) permissions |= QFile::ReadOwner;
        if (ui->permUserWriteBox->isChecked()) permissions |= QFile::WriteOwner;
        if (ui->permUserExeBox->isChecked()) permissions |= QFile::ExeOwner;
        if (ui->permGroupReadBox->isChecked()) permissions |= QFile::ReadGroup;
        if (ui->permGroupWriteBox->isChecked()) permissions |= QFile::WriteGroup;
        if (ui->permGroupExeBox->isChecked()) permissions |= QFile::ExeGroup;
        if (ui->permOtherReadBox->isChecked()) permissions |= QFile::ReadOther;
        if (ui->permOtherWriteBox->isChecked()) permissions |= QFile::WriteOther;
        if (ui->permOtherExeBox->isChecked()) permissions |= QFile::ExeOther;

        file.setPermissions(permissions);
        qDebug() << "Saving permissions" << file.error();

        // Abort on error
        if (file.error()) {
            QMaemo5InformationBox::information(this, tr("Error saving permissions"));
            return;
        }
    }

    QString target = info.absolutePath() + '/' + ui->nameEdit->text();

    // Rename only if allowed and needed
    if (ui->nameEdit->isEnabled() && info.absoluteFilePath() != target) {
        if (QFileInfo(target).exists()) {
            // Such file already exists, abort
            QMaemo5InformationBox::information(this, tr("Filename in use"));
            return;
        } else {
            file.rename(target);
            qDebug() << "Renaming" << file.error();

            // Abort on error
            if (file.error()) {
                QMaemo5InformationBox::information(this, tr("Error renaming file"));
                return;
            }
        }
    }

    this->close();
}
