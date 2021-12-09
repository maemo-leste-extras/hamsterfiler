#include "applicationdialog.h"

ApplicationDialog::ApplicationDialog(QWidget *parent, QString name, QString command) :
    QDialog(parent)
{
    this->setWindowTitle(tr("Edit application"));

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(5);
    this->setLayout(mainLayout);

    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Name");
    nameEdit->setText(name);

    commandEdit = new QLineEdit(this);
    commandEdit->setPlaceholderText("Command");
    commandEdit->setText(command);

    QPushButton *saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(accept()));

    mainLayout->addWidget(nameEdit, 0, 0);
    mainLayout->addWidget(saveButton, 0, 1);
    mainLayout->addWidget(commandEdit, 1, 0, 1, 2);
}
