#include "createdialog.h"

CreateDialog::CreateDialog(QString currentPath, QWidget *parent) :
    QDialog(parent)
{
    this->currentPath = currentPath;

    this->setWindowTitle(tr("Create"));

    QGridLayout *mainLayout = new QGridLayout(this);

    nameEdit = new QLineEdit(this);
    nameEdit->setValidator(new QRegExpValidator(QRegExp("[^/]+"), this));

    fileButton = new QPushButton(tr("File"), this);
    directoryButton = new QPushButton(tr("Directory"), this);

    mainLayout->addWidget(nameEdit, 0, 0, 1, 2);
    mainLayout->addWidget(fileButton, 1, 0);
    mainLayout->addWidget(directoryButton, 1, 1);

    connect(fileButton, SIGNAL(clicked()), this, SLOT(create()));
    connect(directoryButton, SIGNAL(clicked()), this, SLOT(create()));
}

void CreateDialog::create()
{
    if (nameEdit->text().isEmpty()) {
        QMaemo5InformationBox::information(this, tr("Filename cannot be empty"));
    } else {
        QString targetPath = currentPath + '/' + nameEdit->text();

        if (QFile(targetPath).exists()) {
            QMaemo5InformationBox::information(this, tr("Filename already in use"));
        } else {
            if (this->sender() == fileButton) {
                QFile newFile(targetPath);
                if (newFile.open(QFile::WriteOnly))
                    this->close();
                else
                    QMaemo5InformationBox::information(this, tr("File could not be created"));
                newFile.close();
            } else { // this->sender() == directoryButton
                if (QDir().mkdir(targetPath))
                    this->close();
                else
                    QMaemo5InformationBox::information(this, tr("Directory could not be created"));
            }
        }
    }
}
