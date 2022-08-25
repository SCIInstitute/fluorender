#include "QvisTreeWidget.h"
#include "QtCore/qmimedata.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include <iostream>

QvisTreeWidget::QvisTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

QvisTreeWidget::~QvisTreeWidget()
{
}

// Index into the item data where the type located. See the
// QvisTreeWidget::candrop method for details on how they are used.
void QvisTreeWidget::setItemTypeIndex(int index)
{
    mItemTypeIndex = index;
}

int  QvisTreeWidget::getItemTypeIndex() const
{
    return mItemTypeIndex;
}

// Index into the item data where the types accepted are located.
// QvisTreeWidget::candrop method for details on how they are used.
void QvisTreeWidget::setItemAcceptedTypeIndex(int index)
{
    mItemAcceptedIndex = index;
}

int  QvisTreeWidget::getItemAcceptedTypeIndex() const
{
    return mItemAcceptedIndex;
}


void QvisTreeWidget::mousePressEvent(QMouseEvent* aEvent)
{
    QTreeWidget::mousePressEvent(aEvent);
}

void QvisTreeWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
//              << aEvent->position().toPoint().x() << "  "
//              << aEvent->position().toPoint().y() << "  "
//              << std::endl;

    QTreeWidget::mouseMoveEvent(aEvent);
}

void QvisTreeWidget::mouseReleaseEvent(QMouseEvent* aEvent)
{
    QTreeWidget::mouseReleaseEvent(aEvent);

    // Get the item for this mouse event.
    const QPoint clickedPosition = aEvent->pos();
    QTreeWidgetItem * item = itemAt(clickedPosition);

    // It can happen that the mouse event is not on an item but
    // an item is still selected.
    if(item == nullptr)
    {
        QList<QTreeWidgetItem *> items = selectedItems();

        if(items.size() == 1)
            item = items[0];
        else
            return;
    }

    // A tree widget can have mulitple columns so check each one.
    for(int col=0; col<columnCount(); ++col)
    {
        const QIcon icon = item->icon(col);

        if(icon.isNull())
            continue;

        // Get the model index which makes it possible to get the item
        // retangle on a per column basis.
        const QModelIndex modelIndex = indexFromItem(item, col);
        const QRect baseRectangle = visualRect(modelIndex);

        // iconSize() is not always set. As such, get a pixmap that would be
        // generated for the icon given the height of the baseRectangle.
        const QSize iSize(icon.pixmap(baseRectangle.height()).size());
        const int iOffset = baseRectangle.height() - iSize.height();

        // The resulting rectangle is offset towards to origin. That is,
        // the rectangle does not extend below the text. As such, the user
        // must click further up and left than expected.
        QRect iRectangle;
        iRectangle.setTopLeft(baseRectangle.topLeft() + QPoint(iOffset, iOffset));
        iRectangle.setWidth(iSize.width());
        iRectangle.setHeight(iSize.height());

        if(iRectangle.contains(clickedPosition))
        {
            emit iconClicked(item, col);

//            std::cerr << __FUNCTION__ << "  " << __LINE__ << "  icon column " << col << std::endl;
        }
    }
}

void QvisTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget::dragEnterEvent(event);
}

void QvisTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // Determine if the drop indicator can be shown during a paint event.
    QPoint position = event->position().toPoint();

    canDrop(position);

    QTreeWidget::dragMoveEvent(event);
}

void QvisTreeWidget::dropEvent(QDropEvent *event)
{
    if (event->source() == this &&
        (event->dropAction() == Qt::MoveAction ||
         dragDropMode() == QAbstractItemView::InternalMove))
    {
        QPoint position = event->position().toPoint();

        // If the item cannot be dropped ignore the event.
        if(!canDrop(position))
        {
            event->ignore();
            return;
        }
    }

    // Construct two index lists. The first list contains the items selected
    // (e.g. moved). The second list contains the item's original parent.
    QList<QTreeWidgetItem *> children;
    QList<QTreeWidgetItem *> parents;

    const QList<QModelIndex> indexes = selectedIndexes();
    for(const auto & index : indexes)
    {
        // The selectedIndexes returns all colums selected. Only record for column 0 as all
        // columns are moved together. Otherwise signals are emited for each column.
        if(index.column())
            continue;

        QTreeWidgetItem *child  = itemFromIndex(index);
        QTreeWidgetItem *parent = child->parent();

        children.append(child);
        parents.append(parent);
    }

    QTreeWidget::dropEvent(event);

    // After the child has been moved emit a signal.
    for(int i=0; i<children.size(); ++i)
        emit itemMoved(children[i], 0, parents[i], 0);
}

void QvisTreeWidget::paintEvent(QPaintEvent * event)
{
    // Get the drop indicator current state.
    bool show = showDropIndicator();

    // If the drop indicator is currently shown during a paint event,
    // the use the can drop conditional to really show it.
    if(show)
        setDropIndicatorShown(mCanDrop);

    QTreeWidget::paintEvent(event);

    // Restore the drop indicator state.
    setDropIndicatorShown(show);
}

bool QvisTreeWidget::canDrop(QPoint position)
{
    mCanDrop = true;

    QPersistentModelIndex dropIndex = indexAt(position);

    // Get the drop parent. If the indicator position is above or below, the
    // parent of the item is wanted. If indicator position is on the item then
    // the item is the parent.
    QTreeWidgetItem *parent = nullptr;

    if (dropIndex.parent().isValid() && dropIndex.row() != -1)
    {
        DropIndicatorPosition dropIndicator = dropIndicatorPosition();

        switch (dropIndicator)
        {
        case QAbstractItemView::AboveItem:
        case QAbstractItemView::BelowItem:
            parent = itemFromIndex(dropIndex)->parent();
            break;
        case QAbstractItemView::OnItem:
            parent = itemFromIndex(dropIndex);
            break;
        case QAbstractItemView::OnViewport:
            break;
        }
    }

    if(parent)
    {
        // Two modes. If the mItemAcceptedIndex is set for the drop parent then use that
        // type for determining whether drops are accepted. The drop item's type will have
        // to match it. If it is not set, then use the parent item's current type. The drop
        // item's parent type will have to match it.
        int dropAcceptedType;

        if(mItemAcceptedIndex >= 0)
            dropAcceptedType = parent->data(mItemAcceptedIndex, Qt::UserRole).toInt();
        else if(mItemTypeIndex >= 0)
            dropAcceptedType = parent->data(mItemTypeIndex, Qt::UserRole).toInt();
        else
            return mCanDrop;

        // Loop through all of the items selected and check if the item's parent type is
        // the same as the drop parent type.
        const QList<QModelIndex> indexes = selectedIndexes();

        for(const auto & index : indexes)
        {
            QTreeWidgetItem *item = itemFromIndex(index);

            // Assume there is always an item and a parent item.
            if(item && item->parent())
            {
                int dropType = 0;

                // Two modes. If the mItemAcceptedIndex is set for the drop parent and the
                // mItemTypeIndex is set for the item then use the item's type. If only the
                // mItemTypeIndex is set for the item then use the item's parent type.
                if(mItemAcceptedIndex >= 0 && mItemTypeIndex >= 0)
                    dropType = item->data(mItemTypeIndex, Qt::UserRole).toInt();
                else if(mItemTypeIndex >= 0)
                    dropType = item->parent()->data(mItemTypeIndex, Qt::UserRole).toInt();

//                std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << dropType << "  " << dropAcceptedType
//                          << "  " << (dropType & dropAcceptedType)
//                          << std::endl;

                if(dropType && !(dropType & dropAcceptedType))
                {
                    mCanDrop = false;
                    break;
                }
            }
        }
    }
    else
        mCanDrop = false;


//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCanDrop << std::endl;

    return mCanDrop;
}
