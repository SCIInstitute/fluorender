#include "QvisDialogBase.h"

#include <QAction>
#include <QPushButton>
#include <QToolButton>

#include "QvisDragDropToolBar.h"

#include <iostream>

QvisDialogBase::QvisDialogBase(QWidget *parent):
    QDialog(parent)
{
}

QvisDialogBase::~QvisDialogBase()
{
}

void QvisDialogBase::closeEvent(QCloseEvent *e)
{
    mToggleViewAction->setChecked(false);

    QDialog::closeEvent(e);
}

QAction * QvisDialogBase::toggleViewAction()
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

void QvisDialogBase::setToggleViewAction(QAction * toggleViewAction)
{
    mToggleViewAction = toggleViewAction;
}

void QvisDialogBase::toggleView(bool show)
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

QvisDragDropToolBar * QvisDialogBase::dragDropToolBar()
{
    if(mDragDropToolbar == nullptr)
    {
        // The toolbar allows the user to customize actions.
        QString title = this->windowTitle() + tr(" toolbar");
        mDragDropToolbar = new QvisDragDropToolBar(title, this, this);
        mDragDropToolbar->setObjectName(this->windowTitle().simplified().remove(" ") + tr("Toolbar"));
        mDragDropToolbar->setMovable(true);
        mDragDropToolbar->setHidden(true);
        mDragDropToolbar->setToggleCheckableActions(true);
        mDragDropToolbar->setBorderCheckableActions(true);
        mDragDropToolbar->setExclusiveCheckableActions(true);

        // Add a tool button to the drag toolbar that brings up the drop (shortcut) toolbar.
        QString label;
        if(mToolbarActionOrientation == Qt::Horizontal)
            label = tr(" Show  \nToolbar");
        else if(mToolbarActionOrientation == Qt::Vertical)
            label = tr("   T\nS o\nh o\no l\nw b\n   a\n   r");

        mToolbarAction = mDragDropToolbar->dragToolBar()->addAction(label, this, SLOT(showDragDropToolBar()));

        // If the drop (shortcut) toolbar button is clicked make this window visible.
        QObject::connect(mDragDropToolbar, SIGNAL(toggleParentView()), this, SLOT(toggleView()));
    }

    return mDragDropToolbar;
}

QAction* QvisDialogBase::addDragAction(QToolButton *button, const char* member, bool checkable)
{
    QAction *action = new QAction(button->text(), this);
    action->setToolTip(button->toolTip());
    QObject::connect(action, SIGNAL(triggered(bool)), this, member);

    if(checkable)
    {
        action->setCheckable(checkable);
        // When triggered update the checked state.
        QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(setChecked(bool)));
        // Use a lambda function to emit the cursor mode state which gets picked by the data manager.
        QObject::connect(action, &QAction::triggered, this, [=] {
            if(action->isChecked())
                emit currentCursorMode(this->windowTitle(), action->text());
            else
                emit currentCursorMode("", "");
        });
    }

    dragDropToolBar()->addDragAction(action);

    button->setHidden(true);
    return action;
}

Qt::Orientation QvisDialogBase::toolbarActionOrientation() const
{
    return mToolbarActionOrientation;
}

void QvisDialogBase::setToolbarActionOrientation(Qt::Orientation val)
{
    if(mToolbarActionOrientation == val)
        return;

    mToolbarActionOrientation = val;

    if(mToolbarActionOrientation == Qt::Horizontal)
        mToolbarAction->setText(" Show  \nToolbar");
    else if(mToolbarActionOrientation == Qt::Vertical)
        mToolbarAction->setText("  T\nS o\nh o\no l\nw b\n  a\n  r");
}

void QvisDialogBase::showDragDropToolBar()
{
    if(mDragDropToolbar)
    {
        mDragDropToolbar->show();
        mDragDropToolbar->toggleDragDialog(true);
    }
}

void QvisDialogBase::sendNotification()
{
    if(notify)
    {
        std::cerr << "QvisDialogBase::sendNotification() "
          << notifyCount++ << std::endl;
    }
}
