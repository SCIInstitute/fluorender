#include "QvisDragDropToolBar.h"
#include "QvisDragDropWidget.h"

#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QXmlStreamWriter>

#include <iostream>

// Create a drag and drop toolbar. The actual toolbar contains a drag and drop widget which
// is where the real drag and drop work occurs. The source is also a drag and drop widget. If
// there is no drag parent a floating dialog will be created which will popup via a signal from the
// drag toolbar. If there is a drag parent present then the drag toolbar is to be imbedded into it
// and a signal is emited to popup the drag parent.
QvisDragDropToolBar::QvisDragDropToolBar(QString name, QWidget *parent, QWidget *dragParent) :
    QToolBar(name, parent)
{
    // The drag and drop toolbar uses a general drop and drop widget that is embedded
    // in to the toolbar. The drag and drop could have been implemented into the toolbar
    // directly but the widget allows for multiple rows via grid layout.

    // Use a key, the this pointer to prevent cross toolbar widget dropping.
    int key = reinterpret_cast<std::uintptr_t>(this);

    // Create a toolbar with a button that shows/hides the dialog that holds the dragable widgets.
    QString title = name;
    title = title.remove(tr("toolbar"), Qt::CaseInsensitive).trimmed();
    mDropAction = this->addAction(title, this, SLOT(toggleDragDialog()));

    // Create the drop widget for toolbar. The CloseAfterIgnore allows the user to drag
    // a button out of the window so to remove it.
    mDropWidget = new QvisDragDropWidget(this);
    mDropWidget->setWindowTitle(name);
    mDropWidget->setDragDropMode(QvisDragDropWidget::AllowDragAndDrop);
    mDropWidget->setKey(key);

    // A trash can so the user can get remove unwanted buttons.
    if(mGarbageCan)
        mDropWidget->addTrashCan();
    // Otherwise if the user moves the button outside the toobar remove (close) it.
    else
        mDropWidget->setIgnoreMode(QvisDragDropWidget::CloseAfterIgnore);

    // If the user moves the button to the original source window remove (close) it.
    mDropWidget->setCopyMode(QvisDragDropWidget::CloseAfterCopy);

    // When user to drags a button out of the window to the source drag  window so to remove it,
    // get the signal so that it added back in the sibbling dialog. Needed when the source action
    // is moved to drag and not kept.
    if(!mKeepSourceAction)
        connect(mDropWidget, SIGNAL(widgetRemoved(QString)), SLOT(widgetRemoved(QString)));
    // Get button clicks from the toolbar drop widget.
    connect(mDropWidget, SIGNAL(widgetClicked(QString)), SLOT(widgetClicked(QString)));

    this->addWidget(mDropWidget);

    // A separator so the end of the toolbar can be seen.
    this->addSeparator();

    // If the orientation of the toolbar changes - change the orienation of the drag widget.
    // Note this connection captures the base class QToolBar own signal.
    connect(this, SIGNAL(orientationChanged(Qt::Orientation)), mDropWidget, SLOT(setOrientation(Qt::Orientation)));

    // If there is no drag parent create a floating drag dialog and layout for the source drag widget.
    // If there is a drag parent then the drag toolbar is to be imbedded into it.
    if(dragParent == nullptr)
    {
        mDragDialog = new QDialog(parent);
        mDragDialog->setWindowTitle(QString("Drag buttons to the '") + title + QString("' toolbar"));
        mDragLayout = new QVBoxLayout(mDragDialog);

        dragParent = mDragDialog;
    }

    // Create the actual source drag widget.
    mDragWidget = new QvisDragDropWidget(dragParent);
    // Becasue sorting may be enabled, allow only drop copy so the user can return unwanted
    // button to the source drag widget. That is no moving of widgets.
    mDragWidget->setDragDropMode(QvisDragDropWidget::AllowDragAndDropCopy);
    mDragWidget->setKey(key);
    mDragWidget->setSorting(mSortSourceAction);

    // Create toolbar and place the drag widget in it so it looks similar to its sibling.
    mDragToolbar = new QToolBar(dragParent);
    mDragToolbar->addWidget(mDragWidget);

    // If a layout was created then a local drag dialog was created so add the widget to the layout.
    if(mDragLayout)
        mDragLayout->addWidget(mDragToolbar);

    // When user drags a button out of the dialog and it is dropped into the sibbing
    // keep it so the user sees all dragable buttons.
    if(mKeepSourceAction)
        mDragWidget->setCopyMode(QvisDragDropWidget::ShowAfterCopy);
    // When user to drags a button out of the dialog and it is dropped into the sibbing
    // remove it so the user only sees the availble dragable buttons.
    else
        mDragWidget->setCopyMode(QvisDragDropWidget::CloseAfterCopy);

    // Get button clicks from the toolbar drag widget.
    connect(mDragWidget, SIGNAL(widgetClicked(QString)), SLOT(widgetClicked(QString)));
}

QvisDragDropToolBar::~QvisDragDropToolBar()
{
}

// For simplicity all adds are run through the insertDragAction.
void QvisDragDropToolBar::addDragAction(QAction* action)
{
    insertDragAction(nullptr, action);
}

QAction* QvisDragDropToolBar::addDragAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addDragAction(action);
    return action;
}

QAction* QvisDragDropToolBar::addDragAction(const QIcon &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addDragAction(action);
    return action;
}

QAction* QvisDragDropToolBar::addDragAction(const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addDragAction(action);
    return action;
}

QAction* QvisDragDropToolBar::addDragAction(const QIcon &icon, const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addDragAction(action);
    return action;
}

void QvisDragDropToolBar::insertDragAction(QAction *before, QAction *action)
{
    QString name = action->text();

    // No duplicate named actions.
    if(mActions.find(name) != mActions.end())
        return;

    QString beforeName(tr(""));
    if(before != nullptr)
        beforeName = before->text();

    // Get the pixmap for the widget.
    QPixmap pixmap;
    if(!action->icon().isNull())
        pixmap = action->icon().pixmap(mPixmapSize, mPixmapSize);

    if(mImmediatelyAddSourceAction)
    {
        // Insert the widget into the toolbar widget.
        QWidget *widget = mDropWidget->insertWidget(beforeName, name, pixmap);
        widget->setToolTip(action->toolTip());
    }

    if(( mImmediatelyAddSourceAction && mKeepSourceAction) ||
       (!mImmediatelyAddSourceAction))
    {
        // Insert the widget into the drag source widget.
        QWidget *widget = mDragWidget->insertWidget(beforeName, name, pixmap);
        widget->setToolTip(action->toolTip());
    }

    mActions[name] = action;
}

void QvisDragDropToolBar::removeDragAction(QAction *action)
{
    QString name = action->text();

    std::map<QString, QAction *>::iterator it = mActions.find(name);
    if (it != mActions.end())
    {
        mActions.erase (it);

        mDropWidget->removeWidget(name);
        mDragWidget->removeWidget(name);
    }
}


QByteArray QvisDragDropToolBar::saveState(int version) const
{
    // Get the names of the labels currently in the drop widget.
    QList<QString> names = mDropWidget->getWdigetNames();

    QByteArray xmldata;
    QXmlStreamWriter s(&xmldata);

    // No spaces allowed in the element name.
    QString name = mDropWidget->windowTitle();
    name.replace(" ", "_");

    // There may be multiple drag-drop tool bars so use the class name with the
    // toolbar window title as a unique identifier - note the underscore, no spaces.
    s.writeStartDocument();
        s.writeStartElement("QvisDragDropToolBar_" + name);
        s.writeAttribute("Version", QString::number(mCurrentVersion));
        s.writeAttribute("UserVersion", QString::number(version));
        s.writeAttribute("Labels", QString::number(names.size()));

        int i = 0; // Save each name with a unique label - note the underscore, no spaces.
        for (const QString &name : qAsConst(names))
            s.writeAttribute(QString("Label_%1").arg(i++), name);

        s.writeEndElement();
    s.writeEndDocument();

    return xmldata;
}

bool QvisDragDropToolBar::restoreState(const QByteArray &state, int version)
{
    if(state.isEmpty())
        return false;

    // Prevent multiple calls as long as state is not restore. This may
    // happen, if QApplication::processEvents() is called somewhere
    if (mRestoringState)
        return false;

    mRestoringState = true;

    QXmlStreamReader s(state);

    // No spaces allowed in the element name.
    QString name = mDropWidget->windowTitle();
    name.replace(" ", "_");

    // Read the start element - note the underscore, no spaces.
    s.readNextStartElement();
    if (s.name() != (QLatin1String("QvisDragDropToolBar_") + name))
        return false;

    // Read the internal version of the save/restore
    bool ok;
    int v = s.attributes().value("Version").toInt(&ok);
    if (!ok || v != mCurrentVersion)
        return false;

    // Read the user version - not used.
    if (!s.attributes().value("UserVersion").isEmpty())
    {
        int v = s.attributes().value("UserVersion").toInt(&ok);
        if (!ok || v != version)
            return false;
    }

    // Read the number of labels.
    int nLabels = s.attributes().value("Labels").toInt(&ok);
    if (!ok || nLabels == 0)
        return false;

    QList<QString> restoredNames;

    // Read each label - note the underscore, no spaces.
    for(int i=0; i<nLabels; ++i)
    {
        QString name = s.attributes().value(QString("Label_%1").arg(i)).toString();

        // Get the action and use it to restore the drop widget.
        if(mActions.find(name) != mActions.end())
        {
            const QAction *action = mActions[name];
            QPixmap pixmap;

            // Get the pixmap for the widget.
            if(!action->icon().isNull())
                pixmap = action->icon().pixmap(mPixmapSize, mPixmapSize);

            // The widgets were added immediately when inserted then they are already
            // in the tool bar so keep track of them so the ones on in the list can be removed.
            if(mImmediatelyAddSourceAction)
            {
                restoredNames.append(name);
            }
            // The widgets were not added immediately then restore them.
            else
            {
                // Insert the widget into the destination widget.
                QWidget *widget = mDropWidget->addWidget(name, pixmap);
                widget->setToolTip(action->toolTip());
            }

            // Remove the widget from the source widget.
            if(mDragWidget->getCopyMode() == QvisDragDropWidget::CloseAfterCopy)
                mDragWidget->removeWidget(name);
        }
    }

    // If widgets were added immediately then those not part of the restoration need
    // to be removed.
    if(mImmediatelyAddSourceAction)
    {
        // Get all of the names in the mDropWidget.
        QList names = mDropWidget->getWdigetNames();

        for (const QString &name : qAsConst(names))
        {
            // If the widget was restored then continue.
            if(restoredNames.contains(name))
                continue;
            // Otherwise delete it
            else
                mDropWidget->removeWidget(name);
        }
    }

    mRestoringState = false;

    return true;
}

void QvisDragDropToolBar::setIcon(QIcon icon)
{
    // Icon seen in the menu for toggling the toolbar visibility.
    toggleViewAction()->setIcon(icon);

    // Icon seen in the toolbar for toggling the source drag widget visibility.
    // The name should be present so not to confuse its purpose vs the short cut actions.
    mDropAction->setIcon(icon);
    this->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

//
void QvisDragDropToolBar::setPixmapSize(int val)
{
    mPixmapSize = val;
}

int  QvisDragDropToolBar::pixmapSize() const
{
    return mPixmapSize;
}

//
void QvisDragDropToolBar::setMaxColumns(int val)
{
    if(mMaxColumns == val)
        return;

    mMaxColumns = val;

    // ??? Note: only the drag widget can have multiple rows. The
    // ??? drop toolbar is a single row.
    mDragWidget->setMaxColumns(val);
    mDropWidget->setMaxColumns(val);
}

int  QvisDragDropToolBar::maxColumns() const
{
    return mMaxColumns;
}

//
bool QvisDragDropToolBar::sortSourceAction() const
{
    return mSortSourceAction;
}

void QvisDragDropToolBar::setSortSourceAction(bool val)
{
    if(mSortSourceAction == val)
        return;

    mSortSourceAction = val;

    mDragWidget->setSorting(val);
}

// When adding a new action to the source drag widget, immediately add it to the toolbar.
bool QvisDragDropToolBar::immediatelyAddSourceAction() const
{
    return mImmediatelyAddSourceAction;
}

void QvisDragDropToolBar::setImmediatelyAddSourceAction(bool val)
{
    mImmediatelyAddSourceAction = val;
}

// When adding an action to the toolbar, keep the action in the source drag widget.
bool QvisDragDropToolBar::keepSourceAction() const
{
    return mKeepSourceAction;
}

void QvisDragDropToolBar::setKeepSourceAction(bool val)
{
    if(mKeepSourceAction == val)
        return;

    mKeepSourceAction = val;

    QList<QString> names = mDropWidget->getWdigetNames();

    // Get all of the widgets currently in the mDropWidget and copy them back
    // into source mDragWidget.
    if(mKeepSourceAction)
    {
        for (const QString &name : qAsConst(names))
        {
            QAction *action = mActions[name];
            QWidget *widget = nullptr;

            if(action->icon().isNull())
            {
                // Insert the widget in the source widget.
                widget = mDragWidget->insertWidget("", name);
            }
            else
            {
                // Get the pixmap for the widget.
                QPixmap pixmap = action->icon().pixmap(mPixmapSize, mPixmapSize);

                // Insert the widget into the source widget.
                widget = mDragWidget->insertWidget("", name, pixmap);
            }

            widget->setToolTip(action->toolTip());
        }
    }
    // Get all of the widgets in the destination mDropWidget and remove them from
    // the source mDragWidget.
    else
    {
        for (const QString &name : qAsConst(names))
            mDragWidget->removeWidget(name);
    }
}

// Some actions may be used by widgets outside of the toolbar which one wants to be toggled.
// But as part of a toolbar shortcut it may be desirable to have those actions always be true.
bool QvisDragDropToolBar::toggleCheckableActions() const
{
    return mToggleCheckableActions;
}

void QvisDragDropToolBar::setToggleCheckableActions(bool val)
{
    mToggleCheckableActions = val;
}

bool QvisDragDropToolBar::borderCheckableActions() const
{
    return mBorderCheckableActions;
}

void QvisDragDropToolBar::setBorderCheckableActions(bool val)
{
    mBorderCheckableActions = val;
}

bool QvisDragDropToolBar::exclusiveCheckableActions() const
{
    return mExclusiveCheckableActions;
}

void QvisDragDropToolBar::setExclusiveCheckableActions(bool val)
{
    mExclusiveCheckableActions = val;
}

// Display a garbage can for removing an action for the toolbar.
bool QvisDragDropToolBar::garbageCan() const
{
    return mGarbageCan;
}

void QvisDragDropToolBar::setGarbageCan(bool val)
{
    if(mGarbageCan == val)
        return;

    mGarbageCan = val;

    // A trash can so the user can get rid of unwanted buttons.
    if(mGarbageCan)
    {
        mDropWidget->addTrashCan();
        mDropWidget->setIgnoreMode(QvisDragDropWidget::ShowAfterIgnore);
    }
    // If the user moves the button outside the toobar close it.
    else
    {
        mDropWidget->removeTrashCan();
        mDropWidget->setIgnoreMode(QvisDragDropWidget::CloseAfterIgnore);
    }
}

QToolBar * QvisDragDropToolBar::dragToolBar() const
{
    return mDragToolbar;
}

void QvisDragDropToolBar::toggleDragDialog(bool show)
{
    // If there is a local drag dialog make it visible so the user can get to the other actions.
    if(mDragDialog)
    {
        // Slot to hide/show the button drag dialog.
        if(show || mDragDialog->isHidden())
        {
            mDragDialog->show();
            mDragDialog->raise();
        }
        else
        {
            mDragDialog->hide();
        }
    }
    // Otherwise emit a signal to the parent that owns the drag widget to make itself visible
    // so the user can get to the other actions.
    else
    {
        emit toggleParentView();
    }
}

void QvisDragDropToolBar::widgetClicked(const QString name)
{
    // When a action icon is clicked emit the associated action.
    if(mActions.find(name) != mActions.end())
    {
        QAction *action = mActions[name];

        // Some actions may be used by widgets outside of the toolbar which one wants to be toggled.
        // But as part of a toolbar shortcut it may be desirable to have those actions always be true.
        // As such, only toggle when both conditions are true.
        if(mToggleCheckableActions && action->isCheckable())
            emit action->triggered(!action->isChecked());
        else
            emit action->triggered(true);

        emit actionTriggered(action);

        // Normally a checkable button has a highlighted background. That can be hard to see.
        // Instead draw a black border around it.
        if(mBorderCheckableActions && action->isCheckable())
        {
            mDragWidget->setWidgetBorder(name, action->isChecked(), mExclusiveCheckableActions);
            mDropWidget->setWidgetBorder(name, action->isChecked(), mExclusiveCheckableActions);
        }
    }
}

void QvisDragDropToolBar::widgetRemoved(const QString name)
{
    // Get the action and use it to restore the drag widget.
    if(mActions.find(name) != mActions.end())
    {
        const QAction *action = mActions[name];
        QWidget *widget = nullptr;

        if(action->icon().isNull())
        {
            // Add the widget back into the source widget.
            widget = mDragWidget->addWidget(name);
        }
        else
        {
            // Get the pixmap for the widget.
            QPixmap pixmap = action->icon().pixmap(mPixmapSize, mPixmapSize);

            // Add the widget back into the source widget.
            widget = mDragWidget->addWidget(name, pixmap);
        }

        widget->setToolTip(action->toolTip());

        // Make the dialog on top so that the user can see the added widget.
        if(mDragDialog && !mDragDialog->isHidden())
            mDragDialog->raise();
    }
}
