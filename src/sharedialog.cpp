#include "sharedialog.h"

ShareDialog::ShareDialog(QStringList files, QWidget *parent) :
    QDialog(parent)
{
    this->files = files;

    this->setWindowTitle(tr("Share"));

    QHBoxLayout *mainLayout = new QHBoxLayout();
    this->setLayout(mainLayout);

    QPushButton *bluetoothButton = new QPushButton(tr("Bluetooth"), this);
    QPushButton *emailButton = new QPushButton(tr("Email"), this);

    mainLayout->addWidget(bluetoothButton);
    mainLayout->addWidget(emailButton);

    connect(bluetoothButton, SIGNAL(clicked()), this, SLOT(onBluetoothClicked()));
    connect(emailButton, SIGNAL(clicked()), this, SLOT(onEmailClicked()));
}

void ShareDialog::onBluetoothClicked()
{
    for (int i = 0; i < files.size(); i++)
        files[i] = QUrl::fromLocalFile(files.at(i)).toEncoded();

    QDBusInterface("com.nokia.icd_ui",
                   "/com/nokia/bt_ui",
                   "com.nokia.bt_ui",
                   QDBusConnection::systemBus())
    .call("show_send_file_dlg", files);

    this->close();
}

void ShareDialog::onEmailClicked()
{
    for (int i = 0; i < files.size(); i++)
        files[i] = QUrl::fromLocalFile(files.at(i)).toEncoded();

    QDBusInterface("com.nokia.modest",
                   "/com/nokia/modest",
                   "com.nokia.modest",
                   QDBusConnection::sessionBus())
    .call("ComposeMail",
           QString(), // to
           QString(), // cc
           QString(), // bcc
           QString(), // subject
           QString(), // body
           files.join(","));

    this->close();
}
