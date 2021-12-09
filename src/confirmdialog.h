#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QMessageBox>
#include <QAbstractButton>
#include <QKeyEvent>

class ConfirmDialog : public QMessageBox
{
    Q_OBJECT

public:
    enum Type {
        ClearApplications,
        ClearBookmarks,
        DeleteSelected,
        ForceAbort
    };

    ConfirmDialog(QWidget *parent, Type type) :
        QMessageBox(parent)
    {
        this->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        this->button(QMessageBox::Yes)->setText(tr("Yes"));
        this->button(QMessageBox::No)->setText(tr("No"));
        this->setWindowTitle(" ");

        switch (type) {
            case ClearApplications:
                this->setText(tr("Remove all applications?"));
                break;

            case ClearBookmarks:
                this->setText(tr("Remove all bookmarks?"));
                break;

            case DeleteSelected:
                this->setText(tr("Delete selected files?"));
                break;

            case ForceAbort:
                this->setText(tr("Unfinished operations will be forcibly aborted. Continue?"));
                break;
        }
    }

protected:
    void keyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            case Qt::Key_Y:
                this->button(QMessageBox::Yes)->animateClick(100);
                break;

            case Qt::Key_N:
                this->button(QMessageBox::No)->animateClick(100);
                break;

            case Qt::Key_Backspace:
                this->button(QMessageBox::Cancel)->click();
                break;
        }
    }
};

#endif // CONFIRMDIALOG_H
