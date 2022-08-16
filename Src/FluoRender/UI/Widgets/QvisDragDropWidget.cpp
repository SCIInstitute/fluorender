#include "QvisDragDropWidget.h"
#include "ui_QvisDragDropWidget.h"

#include <QDrag>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>
#include <iostream>

// A generalize drag and drop widget that uses widgets, QLabels with pixmaps to
// mimique a button that can be pressed as well as dragged and dropped.
// If the drag and drop widget accepts move drops, buttons can be moved (reordered) as
// well as accept copy drops from another drag and drop widget.
//
// A key can be provided so to allow drops into the widget only from a widget
// with a matching key.
//
// Currently the widget layout is either horizontal or vertical. Similar to a toolbar.
//
// When a button click occurs a "clicked" signal is emited with the name of the button.
// As such, the button names must be unique. No other button signals are emitted.

// Signals are emitted for drag and drap events.

QvisDragDropWidget::QvisDragDropWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisDragDropWidget)
{
    ui->setupUi(this);

    setAcceptDrops(true);

    setWindowFlag(Qt::WindowStaysOnTopHint);
}

QvisDragDropWidget::~QvisDragDropWidget()
{
    delete ui;
}

// Drag and drops modes. If AllowDrag is false then widget can be moved but not dragged out
// of the window.
QvisDragDropWidget::DragDropModes QvisDragDropWidget::getDragDropMode() const
{
    return mDragDropMode;
}

void QvisDragDropWidget::setDragDropMode(QvisDragDropWidget::DragDropModes mode)
{
    // If mSorting is true then by default AllowDropMove must be false (but not the converse).
    if(mSorting && mode.testFlag(AllowDropMove))
        mode ^= AllowDropMove;

    // If AllowDropMove is true then by default AllowDrag must also be true (but not the converse).
    // As Pink Flyod sang "If you don't AllowDrag, you can't AllowDropMove!"
    if(mode.testFlag(AllowDropMove))
        mode |= AllowDrag;

    mDragDropMode = mode;

    this->setAcceptDrops(mDragDropMode.testAnyFlag(AllowDrop));
}


// Decides on what should happen to the original widget after an ignore drag event.
// It can be restored or closed (deleted).
QvisDragDropWidget::DragDropIgnoreModes QvisDragDropWidget::getIgnoreMode() const
{
    return mIgnoreMode;
}

void QvisDragDropWidget::setIgnoreMode(QvisDragDropWidget::DragDropIgnoreModes mode)
{
    mIgnoreMode = mode;
}

// Decides on what should happen to the original widget after a copy drag event.
// It can be restored or closed (deleted).
QvisDragDropWidget::DragDropCopyModes QvisDragDropWidget::getCopyMode() const
{
    return mCopyMode;
}

void QvisDragDropWidget::setCopyMode(QvisDragDropWidget::DragDropCopyModes mode)
{
    mCopyMode = mode;
}

bool QvisDragDropWidget::trashCan() const
{
    return mTrashCanLabel != nullptr;
}

void QvisDragDropWidget::addTrashCan()
{
    if(mTrashCanLabel == nullptr)
    {
        // Add a trash can widget.
        mTrashCanLabel = new QLabel("Trash", this);
        mTrashCanLabel->setToolTip("Drag widgets here to delete.");
        mTrashCanLabel->setAttribute(Qt::WA_DeleteOnClose);

        // Set the of the pixmap based on the size of the labels.
        int height = 16;

        for (QLabel *widget : qAsConst(mDragWidgets))
            height = std::max(height, widget->height()-2);

//        mTrashCanLabel->setPixmap(QPixmap(":/UI/icons/misc/TrashcanOutline.png").scaledToHeight(height));
        mTrashCanLabel->setPixmap(QPixmap(":/UI/icons/misc/TrashcanSolid.svg").scaledToHeight(height));

        mTrashCanPixmap = mTrashCanLabel->pixmap(Qt::ReturnByValue);

        // Create a temporary pixmap that mimiques a button press.
        mTrashCanTempPixmap = mTrashCanPixmap;
        if(!mTrashCanPixmap.isNull())
        {
            QPainter painter;
            painter.begin(&mTrashCanTempPixmap);
            painter.fillRect(mTrashCanTempPixmap.rect(), QColor(127, 127, 127, 127));
            painter.end();
        }

        updateLayout();
    }
}

void QvisDragDropWidget::removeTrashCan()
{
    mTrashCanLabel->close();
    mTrashCanLabel = nullptr;
}

// A key so to allow drops into the widget only from a widget with a matching key.
// If the key value is zero it is ignored.
int QvisDragDropWidget::getKey() const
{
    return mKey;
}

void QvisDragDropWidget::setKey(int key)
{
    mKey = key;
}

// Allows alphabetical sorting of the entries by their name.
bool QvisDragDropWidget::getSorting() const
{
    return mSorting;
}

void QvisDragDropWidget::setSorting(bool sorting)
{
    // If mSorting is true then by default AllowDropMove must be false (but not the converse).
    if(sorting && mDragDropMode.testFlag(AllowDropMove))
        mDragDropMode ^= AllowDropMove;

    mSorting = sorting;

    // Update the widget layout with the sorting.
    if(mSorting)
        updateLayout();
}

void QvisDragDropWidget::setMaxColumns(int val)
{
    if(mMaxColumns == val)
        return;

    mMaxColumns = val;

    updateLayout();
}

int  QvisDragDropWidget::maxColumns()
{
    return mMaxColumns;
}

// Set the orientation - currently widgets are display as single row (default)
// or a single column.
Qt::Orientation QvisDragDropWidget::getOrientation()
{
    return mOrientation;
}

void QvisDragDropWidget::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;

    // Update the widget layout with the new orientation.
    updateLayout();
}

// Get/Add/Remove methods for dragable widgets.
QLabel * QvisDragDropWidget::getWidget(const QString name) const
{
    if(name.isEmpty())
        return nullptr;

    // Find the widget from the list.
    for (QLabel *widget : qAsConst(mDragWidgets))
    {
        if( name == widget->objectName() )
            return widget;
    }

    return nullptr;
}

QLabel* QvisDragDropWidget::addWidget(const QString name, const QString pixmapName)
{
    return insertWidget("", name, pixmapName);
}

QLabel* QvisDragDropWidget::addWidget(const QString name, const QPixmap &pixmap)
{
    return insertWidget("", name, pixmap);
}

QLabel* QvisDragDropWidget::insertWidget(const QString before, const QString name, const QString pixmapName)
{
    QPixmap pixmap;
    if(!pixmapName.isEmpty())
       pixmap = QPixmap(pixmapName);

    return insertWidget(before, name, pixmap);
}

QLabel* QvisDragDropWidget::insertWidget(const QString before, const QString name, const QPixmap &pixmap)
{
    QLabel *label = nullptr;

    if(!name.isEmpty())
        label = getWidget(name);

    if(label != nullptr)
        return label;

    // Create a new label.
    label = new QLabel(name, this);
    if(!pixmap.isNull())
        label->setPixmap(pixmap);
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Raised);
    label->setObjectName(name); // Set the obect name as setting a pixmap removes the text name.
    label->show();
    label->setAttribute(Qt::WA_DeleteOnClose);

    int i = -1;

    QLabel *beforeLabel = getWidget(before);

    if(beforeLabel != nullptr)
        i = mDragWidgets.indexOf(beforeLabel);

    if(i >= 0)
        mDragWidgets.insert(i, label);
    else
        mDragWidgets.append(label);

    // If sorting, sort by name then update the layout.
    if(mSorting)
    {
        std::sort(mDragWidgets.begin(), mDragWidgets.end(),
              [](const QLabel* a, const QLabel* b) -> bool { return a->objectName() < b->objectName(); });
    }

    updateLayout();

    return label;
}

void QvisDragDropWidget::removeWidget(const QString name)
{
    QLabel *widget = getWidget(name);

    if(widget == nullptr)
        return;

    // Remove the widget from the list.
    int i = mDragWidgets.indexOf(widget);

    if(i >= 0)
    {
        mDragWidgets.remove(i);

        // Update the widget layout.
        updateLayout();

        // Closing the label deletes it.
        widget->close();
    }
}

// Return the widget names - used when saving the state.
QList<QString> QvisDragDropWidget::getWdigetNames() const
{
    QList<QString> names;

    for (const QLabel *widget : qAsConst(mDragWidgets))
        names.append(widget->objectName());

    return names;
}

void QvisDragDropWidget::setWidgetBorder(const QString name, bool border, bool exclusive)
{
    for (QLabel *widget : qAsConst(mDragWidgets))
    {
        if(name == widget->objectName())
        {
            if(border)
            {
                widget->setStyleSheet("QLabel {"
                                      "border-style: solid;"
                                      "border-width: 1px;"
                                      "border-color: black; "
                                      "}");
            }
            else
            {
                widget->setStyleSheet("QLabel {"
                                      "border-style: none;"
                                      "}");
            }
        }
        else if(exclusive)
        {
            widget->setStyleSheet("QLabel {"
                                  "border-style: none;"
                                  "}");
        }
    }
}

// Mouse event handling.
void QvisDragDropWidget::mousePressEvent(QMouseEvent *event)
{
    mMouseDownPos = event->position().toPoint();

    // Get the label that was pressed.
    QLabel *label = static_cast<QLabel*>(childAt(mMouseDownPos));

    // Ignore any clicks on the trash can.
    if (!label || label == mTrashCanLabel)
        return;

    mChildLabel = label;

    // Get the original pixmap.
    mChildPixmap = mChildLabel->pixmap(Qt::ReturnByValue);

    // If the pixmap is null then text only so create a pixmap.
    if(mChildPixmap.isNull())
    {
        // Create a pixmap but slightly smaller, QSize(2,2) than
        // the label. Otherwise the label size grows.
        qreal dpr = window()->windowHandle()->devicePixelRatio();
        QPixmap pixmap(mChildLabel->size() * dpr - QSize(2,2));
        pixmap.setDevicePixelRatio(dpr);
        mChildLabel->render(&pixmap);
        mChildLabel->setPixmap(pixmap);

        mChildPixmap = mChildLabel->pixmap(Qt::ReturnByValue);
    }

    // Create a temporary pixmap that mimiques a button press.
    if(!mChildPixmap.isNull())
    {
        QPixmap tempPixmap = mChildPixmap;
        QPainter painter;
        painter.begin(&tempPixmap);
        painter.fillRect(tempPixmap.rect(), QColor(127, 127, 127, 127));
        painter.end();
        mChildLabel->setPixmap(tempPixmap);
    }

    mChildLabel->show();
}

void QvisDragDropWidget::mouseMoveEvent(QMouseEvent *event)
{    
    // Create a drag event only if there is mouse movement so to allow
    // a button press.
    if (mChildLabel && mDragDropMode.testFlag(AllowDrag))
    {
        QPoint point = event->pos() - mMouseDownPos;

        if (point.manhattanLength() > 5)
            createDragEvent(event);
    }
}

void QvisDragDropWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mChildLabel)
    {
        // Mouse released so restore the original pixmap.
        if(!mChildPixmap.isNull())
            mChildLabel->setPixmap(mChildPixmap);
        mChildLabel->show();

        emit widgetClicked(mChildLabel->objectName());
    }

    mChildLabel = nullptr;
}

void QvisDragDropWidget::createDragEvent(QMouseEvent *event)
{
    if (!mChildLabel)
        return;

    // Create a data stream to store the pixmap, object name, and tooltip
    // which are used to create a button on the receiving widget.
    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::WriteOnly);
    dataStream << mKey
               << mChildPixmap
               << mChildLabel->objectName()
               << mChildLabel->toolTip();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("QvisDragDropWidget/labelData", byteArray);

    // Create a drag object to store the data and an icon using the original pixmap.
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->position().toPoint() - mChildLabel->pos());
    if(!mChildPixmap.isNull())
        drag->setPixmap(mChildPixmap);

    // Start the drag event and look for a drop action.
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

    if(dropAction == Qt::CopyAction)
        emit widgetCopied(mChildLabel->objectName());

    // If an ignore or copy action with a show mode or move action, restore the original widget.
    if((dropAction == Qt::IgnoreAction && mIgnoreMode == ShowAfterIgnore) ||
       (dropAction == Qt::CopyAction   && mCopyMode   == ShowAfterCopy) ||
       (dropAction == Qt::MoveAction))
    {
        if(mChildLabel)
        {
            if(!mChildPixmap.isNull())
                mChildLabel->setPixmap(mChildPixmap);

            mChildLabel->show();
        }
    }
    // If an ignore or copy action with a close mode, close the original widget.
    else if((dropAction == Qt::IgnoreAction && mIgnoreMode == CloseAfterIgnore) ||
            (dropAction == Qt::CopyAction   && mCopyMode   == CloseAfterCopy))
    {
        emit widgetRemoved(mChildLabel->objectName());

        // Remove the widget which updates the layout and closes the widget.
        removeWidget(mChildLabel->objectName());
    }

    if(mTrashCanLabel)
    {
    }

    mChildLabel = nullptr;
}

void QvisDragDropWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("QvisDragDropWidget/labelData"))
    {
        QByteArray byteArray = event->mimeData()->data("QvisDragDropWidget/labelData");
        QDataStream dataStream(&byteArray, QIODevice::ReadOnly);

        int key;
        QPixmap pixmap;
        QString name, toolTip;
        dataStream >> key >> pixmap >> name >> toolTip;

        // If a key is present only accept drag events from those widgets with the same key.
        if(key && key != mKey)
        {
            event->ignore();
        } else if (event->source() == this) {
            if(acceptDrops())
                event->setDropAction(Qt::MoveAction);
            else
                event->setDropAction(Qt::IgnoreAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void QvisDragDropWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("QvisDragDropWidget/labelData")) {
        QByteArray byteArray = event->mimeData()->data("QvisDragDropWidget/labelData");
        QDataStream dataStream(&byteArray, QIODevice::ReadOnly);

        int key;
        QPixmap pixmap;
        QString name, toolTip;
        dataStream >> key >> pixmap >> name >> toolTip;

        // If a key is present only accept drag events from those widgets with the same key.
        if(key && key != mKey)
        {
            event->ignore();
        }
        else if (event->source() == this)
        {
            if(acceptDrops())
            {
                // Get the label that was pressed.
                QLabel *label = static_cast<QLabel*>(childAt(event->position().toPoint()));

                // If over the trash can highlight.
                if (label && label == mTrashCanLabel)
                {
                    // Get the original pixmap.
                    mMouseOverTrashCan = true;

                    mTrashCanLabel->setPixmap(mTrashCanTempPixmap);
                    mTrashCanLabel->show();
                }
                else
                {
                    if(mMouseOverTrashCan)
                    {
                        mTrashCanLabel->setPixmap(mTrashCanPixmap);
                        mTrashCanLabel->show();
                        mMouseOverTrashCan = false;
                    }
                }

                event->setDropAction(Qt::MoveAction);
            }
            else
                event->setDropAction(Qt::IgnoreAction);

            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void QvisDragDropWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("QvisDragDropWidget/labelData"))
    {
        QByteArray byteArray = event->mimeData()->data("QvisDragDropWidget/labelData");
        QDataStream dataStream(&byteArray, QIODevice::ReadOnly);

        int key;
        QPixmap pixmap;
        QString name, toolTip;
        dataStream >> key >> pixmap >> name >> toolTip;

        // If a key is present only accept drop events from those widgets with the same key.
        if(key && key != mKey)
        {
            event->ignore();
            return;
        }

        // If moving wthin the same widget check for a garbage can.
        if (event->source() == this)
        {
            if(mTrashCanLabel)
            {
                QLabel *label = static_cast<QLabel*>(childAt(event->position().toPoint()));

                if(label == mTrashCanLabel)
                {
                    if(mMouseOverTrashCan)
                    {
                        mTrashCanLabel->setPixmap(mTrashCanPixmap);
                        mTrashCanLabel->show();
                        mMouseOverTrashCan = false;
                    }

                    emit widgetRemoved(mChildLabel->objectName());

                    // Remove the widget which updates the layout and closes the widget.
                    removeWidget(mChildLabel->objectName());

                    mChildLabel = nullptr;

                    event->setDropAction(Qt::MoveAction);
                    event->accept();

                    return;
                }
            }
        }
        // Do not add the same widget twice.
        else
        {
            for (const QLabel *widget : qAsConst(mDragWidgets))
            {
                if(widget->objectName() == name)
                {
                    event->ignore();
                    return;
                }
            }
        }

        // The start index is the index of the first widget in the row (horizontal layout).
        int startIndex = 0;

        // If there are multiple rows (horizontal layout), get the row for the widget.
        if(mMaxColumns && mDragWidgets.size() > mMaxColumns)
        {
            // The gap is half the distance between two rows of widgets.
            int gap = 0;

            if(mOrientation == Qt::Horizontal)
                gap = (mDragWidgets[mMaxColumns]->pos().y() - (mDragWidgets[0]->pos().y() + mDragWidgets[0]->height())) / 2;
            else
                gap = (mDragWidgets[mMaxColumns]->pos().x() - (mDragWidgets[0]->pos().x() + mDragWidgets[0]->width())) / 2;

            if(mOrientation == Qt::Horizontal)
                std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
                          << gap << "  " << mDragWidgets[mMaxColumns]->pos().y() << "  " << mDragWidgets[0]->pos().y()  << "  " << mDragWidgets[0]->height()
                          << std::endl;
            else
                std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
                          << gap << "  " << mDragWidgets[mMaxColumns]->pos().x() << "  " << mDragWidgets[0]->pos().x()  << "  " << mDragWidgets[0]->width()
                          << std::endl;

            // Get the row (horizontal layout).
            while(startIndex < mDragWidgets.size())
            {
                if(mOrientation == Qt::Horizontal)
                    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
                              << startIndex << "  " << event->position().y() << "  " << mDragWidgets[0]->pos().y()  << "  " << mDragWidgets[0]->height()
                              << std::endl;
                else
                    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
                              << startIndex << "  " << event->position().x() << "  " << mDragWidgets[0]->pos().x()  << "  " << mDragWidgets[0]->width()
                              << std::endl;

                // If the position is above the top (horizontal layout) of the widget then continue.
                if((mOrientation == Qt::Horizontal &&
                    event->position().y() > mDragWidgets[startIndex]->pos().y() + mDragWidgets[startIndex]->height() + gap) ||
                   (mOrientation == Qt::Vertical   &&
                    event->position().x() > mDragWidgets[startIndex]->pos().x() + mDragWidgets[startIndex]->width()  + gap))
                    startIndex += mMaxColumns;
                else
                    break;
            }

            if(startIndex > mDragWidgets.size())
                startIndex = int(mDragWidgets.size() / mMaxColumns) * mMaxColumns;
        }

        // Get the column (horizontal layout) of the new widget.
        int newIndex = startIndex;
        while(newIndex < mDragWidgets.size() && (!mMaxColumns || newIndex < startIndex+mMaxColumns))
        {
            QLabel *widget = mDragWidgets[newIndex];

            // If the position is after the middle of a widget then continue.
            if((mOrientation == Qt::Horizontal &&
                event->position().x() > widget->pos().x() + widget->width()/2) ||
               (mOrientation == Qt::Vertical   &&
                event->position().y() > widget->pos().y() + widget->height()/2))
                ++newIndex;
            else
                break;
        }

        // Moving the widget within the window.
        if (event->source() == this)
        {
            if(mDragDropMode.testFlag(AllowDropMove) && !mSorting)
            {
                // Get the current index of the widget
                int oldIndex = mDragWidgets.indexOf(mChildLabel);

                std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << oldIndex << "  " << newIndex << std::endl;

                // When moving a widget forward account for itself by subtracking one.
                if(oldIndex < newIndex)
                    newIndex -= 1;

                std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << oldIndex << "  " << newIndex << std::endl;

                // If different move the widget and update the layout.
                if(newIndex != oldIndex)
                {
                    mDragWidgets.move(oldIndex, newIndex);
                    updateLayout();

                    emit widgetMoved(name);
                }

                event->setDropAction(Qt::MoveAction);
                event->accept();
            }
            else
            {
                event->ignore();
            }
        }
        // Dropped in a new widget.
        else if( mDragDropMode.testFlag(AllowDropCopy))
        {
            // Create a new label widget.
            QLabel *label = new QLabel(name, this);
            if(!pixmap.isNull())
                label->setPixmap(pixmap);
            label->setFrameShape(QFrame::Panel);
            label->setFrameShadow(QFrame::Raised);
            label->setObjectName(name); // Set the obect name as setting a pixmap removes the text name.
            label->setToolTip(toolTip);
            label->show();
            label->setAttribute(Qt::WA_DeleteOnClose);

            mDragWidgets.insert(newIndex, label);

            // If sorting, sort by name then update the layout.
            if(mSorting)
            {
                std::sort(mDragWidgets.begin(), mDragWidgets.end(),
                      [](const QLabel* a, const QLabel* b) -> bool { return a->objectName() < b->objectName(); });
            }

            // Update the recieving widget layout.
            updateLayout();

            emit widgetAdded(name);

            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
}

void QvisDragDropWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if(mTrashCanLabel)
    {
        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << windowTitle().toStdString() << std::endl;

        // Set the of the pixmap based on the size of the labels.
        if(mDragWidgets.size())
        {
            int height = 0;

            for (QLabel *widget : qAsConst(mDragWidgets))
                height = std::max(height, widget->height()-2);

//            mTrashCanLabel->setPixmap(QPixmap(":/UI/icons/misc/trash-can-icon-28692.png").scaledToHeight(height));
            mTrashCanLabel->setPixmap(QPixmap(":/UI/icons/misc/trash-solid.svg").scaledToHeight(height));

            mTrashCanPixmap = mTrashCanLabel->pixmap(Qt::ReturnByValue);

            // Create a temporary pixmap that mimiques a button press.
            mTrashCanTempPixmap = mTrashCanPixmap;
            if(!mTrashCanPixmap.isNull())
            {
                QPainter painter;
                painter.begin(&mTrashCanTempPixmap);
                painter.fillRect(mTrashCanTempPixmap.rect(), QColor(127, 127, 127, 127));
                painter.end();
            }
        }
    }
}

void QvisDragDropWidget::updateLayout()
{
    // Remove all widgets from the layout.
    for (QLabel *widget : qAsConst(mDragWidgets))
        ui->widgetLayout->removeWidget(widget);

    // Add the widgets back to the layout.
    int i = 0, row = 0, col = 0;
    for (QLabel *widget : qAsConst(mDragWidgets))
    {
        if(mMaxColumns  > 0)
        {
            row = i / mMaxColumns;
            col = i % mMaxColumns;
        }
        else
        {
            row = 0;
            col = i;
        }

        ++i;

        if(mOrientation == Qt::Horizontal)
            ui->widgetLayout->addWidget(widget, row, col);
        else
            ui->widgetLayout->addWidget(widget, col, row);
    }

    if(mTrashCanLabel)
    {
        ui->widgetLayout->removeWidget(mTrashCanLabel);

        if(mOrientation == Qt::Horizontal)
            ui->widgetLayout->addWidget(mTrashCanLabel, 0, i++);
        else
            ui->widgetLayout->addWidget(mTrashCanLabel, i++, 0);

        mDragWidgets.size() ? mTrashCanLabel->show() :  mTrashCanLabel->hide();
    }
}
