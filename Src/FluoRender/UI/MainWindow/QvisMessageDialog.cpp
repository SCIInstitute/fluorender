#include "QvisMessageDialog.h"
#include "ui_QvisMessageDialog.h"

#include <QAction>

QvisMessageDialog::QvisMessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QvisMessageDialog)
{
    ui->setupUi(this);
}

QvisMessageDialog::~QvisMessageDialog()
{
    delete ui;
}

QAction * QvisMessageDialog::toggleViewAction()
{
    if(mToggleViewAction == nullptr)
    {
        mToggleViewAction = new QAction(this->windowTitle(), this);
        mToggleViewAction->setCheckable(true);
        mToggleViewAction->setData(QString("Internal"));
        connect(mToggleViewAction, SIGNAL(triggered(bool)), this, SLOT(toggleView(bool)));
    }

    return mToggleViewAction;
};

void QvisMessageDialog::setToggleViewAction(QAction * toggleViewAction)
{
    mToggleViewAction = toggleViewAction;
}

void QvisMessageDialog::postInformationalMessage(QString message)
{
    toggleView(true);

    ui->MessageText->clear();
    ui->MessageText->setTextColor(Qt::black);
    ui->MessageText->setText(message);
}

void QvisMessageDialog::postWarningMessage(QString message)
{
    toggleView(true);

    ui->MessageText->clear();
    ui->MessageText->setTextColor(Qt::yellow);
    ui->MessageText->setText(message);
}

void QvisMessageDialog::postErrorMessage(QString message)
{
    toggleView(true);

    ui->MessageText->clear();
    ui->MessageText->setTextColor(Qt::red);
    ui->MessageText->setText(message);
}

void QvisMessageDialog::toggleView(bool show)
{
    // If mToggleViewAction is internal then make the dialog visibile/hidden.
    if(mToggleViewAction)
    {
        if(mToggleViewAction->data().isValid() &&
           mToggleViewAction->data().toString() == "Internal")
        {
            const QSignalBlocker blocker(mToggleViewAction);
            if(show || !mToggleViewAction->isCheckable())
            {
                this->show();
                this->raise();
                mToggleViewAction->setChecked(true);
            }
            else
            {
                this->hide();
                mToggleViewAction->setChecked(false);
            }
        }
        // Otherwise emit a signal.
        else
        {
            emit mToggleViewAction->triggered(show);
        }
    }
}

void QvisMessageDialog::clearMessage()
{
    ui->MessageText->clear();
}
