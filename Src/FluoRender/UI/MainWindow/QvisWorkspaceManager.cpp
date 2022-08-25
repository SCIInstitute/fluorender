#include "QvisWorkspaceManager.h"
#include "ui_QvisWorkspaceManager.h"

#include "QvisDatasetManager.h"
#include "DataTreeNode.h"

#include "QtCore/qsize.h"
#include "QtCore/qpoint.h"

#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSignalMapper>
#include <QTreeWidgetItem>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>

// QvisWorkspaceManager
QvisWorkspaceManager::QvisWorkspaceManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisWorkspaceManager)
{
    ui->setupUi(this);

    mAgent = new WorkspaceAgent();

    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    // Set the item type index which will restrict drops to being only to
    // parent items with the same type as the item's parent type. Thus volumes
    // can be moved to other volume groups. Similarly meshes can be moved
    // to other mesh groups. Similarly volume and mesh groups can be move to
    // other view windows. Reordering is also possible.
    ui->WorkspaceTreeWidget->setItemTypeIndex(ITEM_DATA_TYPE);

    // Create an initial node for the tree
    WorkspaceActiveDatasetsAdd();

    // Add an initial view window
    WorkspaceViewWindowAddClicked();

    ui->WorkspaceTreeWidget->setColumnWidth(0, this->width()/2);

    // Change over to a functor which QtDesigner does not do.
    QObject::connect(ui->VolumeGroupAddButton, &QToolButton::clicked, this, [this]{WorkspaceGroupAddClicked(DatasetAttributes::VolumeGroup);});
    QObject::connect(ui->MeshGroupAddButton,   &QToolButton::clicked, this, [this]{WorkspaceGroupAddClicked(DatasetAttributes::MeshGroup);});
}

QvisWorkspaceManager::~QvisWorkspaceManager()
{
    delete ui;
}

void QvisWorkspaceManager::SetAgent(InterfaceAgent* agent)
{
    mAgent = (WorkspaceAgent *) agent;
}

void QvisWorkspaceManager::updateWindow(bool doAll)
{
    DataTreeNode * node = mAgent->getDataTree();

    if(doAll)
    {
        // Who is responsible for clearing the tree??
        for(int i=0; i<ui->WorkspaceTreeWidget->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem * item = ui->WorkspaceTreeWidget->topLevelItem(i);

            delete this->getItemNode(item);
        }

        ui->WorkspaceTreeWidget->clear();

        // Create an initial node for the tree
        QTreeWidgetItem * praentItem = WorkspaceActiveDatasetsAdd(node);

        // Now add all of the children.
        for(const auto childNode : node->children)
            addChildItem(praentItem, childNode);
    }
}

void QvisWorkspaceManager::addChildItem(QTreeWidgetItem * parent, const DataTreeNode *node)
{
    QString name = node->name.c_str();
    QColor color = QColor(node->color[0], node->color[1], node->color[2]);

    QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({name, "", ""}));

    if(node->type & (DatasetAttributes::VolumeGroup | DatasetAttributes::MeshGroup))
    {
        // FIXME Set the index of group.
        child->setFlags(child->flags() | Qt::ItemIsEditable);
    }

    setItemNode(child, node);

    if(node->visible)
        child->setIcon(ITEM_NAME,  GetLightIcon(color));
    else
        child->setIcon(ITEM_NAME,  GetLightIcon(QColor(Qt::black)));

    child->setIcon(ITEM_COLOR, GetColorIcon(color));

    if(node->mask)
        child->setIcon(ITEM_STATUS, QIcon(":Src/FluoRender/UI/icons/misc/Mask.png"));

    if(node->selected)
        mCurrentTreeWidgetItem = child;

    if(node->active && node->type == DatasetAttributes::VolumeGroup)
        mCurrentVolumeGroup = name;
    else if(node->active && node->type == DatasetAttributes::MeshGroup)
        mCurrentMeshGroup = name;
    else if(node->active && node->type == DatasetAttributes::Volume)
        mCurrentVolume = name;
    else if(node->active && node->type == DatasetAttributes::Mesh)
        mCurrentMesh = name;

    parent->addChild(child);

    for(const auto childNode : node->children)
        addChildItem(child, childNode);

    WorkspaceUpdate(NODE_DATA_SELECTED, true);
}

void QvisWorkspaceManager::setDatasetManager(QvisDatasetManager *datasetManager)
{
    mDatasetManager = datasetManager;
}

// Workspace view
void QvisWorkspaceManager::WorkspaceUpdateSelection(const QString name, DatasetAttributes::DatasetType datasetType)
{
    mCurrentTreeWidgetItem = GetTreeWidgetItem(name);

    // If the dataset is not in the tree check if there is a group
    // that can be selected.
    if(mCurrentTreeWidgetItem == nullptr)
    {
        // By selecting the active datasets if there is a single view window, volume
        // or mesh gruop it will be set as current. This step allow something to be
        // selected so the user does not have to do so.
        mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentViewWindow);
        WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);

        if(datasetType == DatasetAttributes::Volume && mCurrentVolumeGroup.size())
            mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentVolumeGroup);
        else if(datasetType == DatasetAttributes::Mesh && mCurrentMeshGroup.size())
            mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentMeshGroup);
    }

    // Select the current item in the tree view.
    WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);
}

void QvisWorkspaceManager::WorkspaceRename(const QString oldName, const QString newName, DatasetAttributes::DatasetType datasetType, const QString fullname)
{
    // Traverse the workspace tree and rename the dataset name(s).
    // Note if a dataset is present multiple times the root name can be
    // renamed for ALL instances.
    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    bool checkBaseName = true;
    bool yesToAll = false;

    // If present, delete the dataset from the tree.
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QTreeWidgetItem *treeItem = *it;
        it++;

        // Only rename datasets with the same type.
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(treeItem->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());
        if(datasetType != dType)
            continue;

        // Only rename datasets with the same base name.
        QString name = treeItem->text(ITEM_NAME);
        QString suffix("");

        // Rename datasets with the same name.
        if(name == oldName)
        {
            suffix = tr("");
        }
        // Change datasets with the same base name and get the suffix "_N" where N is a number.
        else if(checkBaseName && name.startsWith(oldName))
        {
            suffix = name.last(name.size()-oldName.size());

            // Make sure there is an underscore followed by an integer.
            if(!suffix.startsWith("_") || suffix.last(suffix.size()-1).toInt() == 0)
                continue;

            if(yesToAll == false)
            {
                QMessageBox dialog;
                dialog.setText("Datasets that have the same base name, '" + oldName + "' have been found.");
                dialog.setInformativeText("Do you want to rename all of these datasets too?\n\n"
                                          "Ok to just rename '" + name + "'.");
                dialog.setStandardButtons(QMessageBox::Ok | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);

                int value = dialog.exec();

                switch (value) {
                case QMessageBox::Ok:
                    break;

                case QMessageBox::YesToAll:
                    yesToAll = true;
                    break;

                case QMessageBox::NoToAll:
                    checkBaseName = false;
                    continue;

                case QMessageBox::Cancel:
                    continue;

                default:
                    continue;
                }
            }
        }
        // Name does not match.
        else
            continue;

        // Update the tree item with the new name.
        treeItem->setText(ITEM_NAME, newName + suffix);

        // Update the current selection with the new name.
        if(datasetType == DatasetAttributes::Mesh && mCurrentVolume == name)
            mCurrentVolume = newName + suffix;
        else if(datasetType == DatasetAttributes::Volume && mCurrentMesh == name)
            mCurrentMesh = newName + suffix;

        // Update the last selection list with the new name.
        QString viewWindow  = treeItem->parent()->parent()->text(ITEM_NAME);
        QString parentGroup = treeItem->parent()->text(ITEM_NAME);

        mLastSelections[viewWindow].datasets[parentGroup] = newName + suffix;

        emit updateViewWindow(viewWindow);
    }
}

void QvisWorkspaceManager::WorkspaceAdd(const QString parentName, const QString addName, DatasetAttributes::DatasetType datasetType, const QString fullname)
{
    QTreeWidgetItem * parent = nullptr;

    // If a parentName then the signal is from a menu action so use the name to get the parent group.
    if(parentName.size())
    {
        parent = GetTreeWidgetItem(parentName);
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(parent->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

        // If the parent is a view window then no volume or mesh group so create one.
        if(dType == DatasetAttributes::ViewWindow)
        {
            if(mCurrentViewWindow != parentName)
            {
                mCurrentViewWindow = parentName;
                emit activeViewWindowChanged(mCurrentViewWindow);
            }

            if(datasetType == DatasetAttributes::Volume)
                parent = WorkspaceGroupAddClicked(DatasetAttributes::VolumeGroup);
            else if(datasetType == DatasetAttributes::Mesh)
                parent = WorkspaceGroupAddClicked(DatasetAttributes::MeshGroup);
        }
        else if(dType == DatasetAttributes::VolumeGroup && datasetType == DatasetAttributes::Volume)
        {
            mCurrentVolumeGroup = parentName;
        }
        else if(dType == DatasetAttributes::MeshGroup && datasetType == DatasetAttributes::Mesh)
        {
            mCurrentMeshGroup = parentName;
        }
    }
    // No name so from a button click so use the current volume/mesh group.
    else
    {
        if(datasetType == DatasetAttributes::Volume)
            parent = GetTreeWidgetItem(mCurrentVolumeGroup);
        else if(datasetType == DatasetAttributes::Mesh)
            parent = GetTreeWidgetItem(mCurrentMeshGroup);

        // No parent, but if there is current view window with no group then a group can be created.
        if(parent == nullptr && !mCurrentViewWindow.isEmpty())
        {
            QTreeWidgetItem *item = GetTreeWidgetItem(mCurrentViewWindow);

            // No volume or mesh group may be current because there can be mutliple
            // volume or mesh groups and it is abiguous as to which one is current.
            // As such, only create a group if the are no groups.
            int nVolumes = 0;
            int nMeshes = 0;

            for(int i=0; i<item->childCount(); ++i)
            {
                QTreeWidgetItem *child = item->child(i);
                DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

                if(dType == DatasetAttributes::VolumeGroup)
                    ++nVolumes;
                else if(dType == DatasetAttributes::MeshGroup)
                    ++nMeshes;
            }

            // If there is not a group then create one.
            if(datasetType == DatasetAttributes::Volume && nVolumes == 0)
                parent = WorkspaceGroupAddClicked(DatasetAttributes::VolumeGroup);
            else if(datasetType == DatasetAttributes::Mesh && nMeshes == 0)
                parent = WorkspaceGroupAddClicked(DatasetAttributes::MeshGroup);
        }
    }

    // Have a parent group so can now add the dataset.
    if(parent)
    {
        if(datasetType & (DatasetAttributes::Volume | DatasetAttributes::Mesh))
        {
            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);
            QString name = WorkspaceExists(addName, datasetType);
            QColor color = QColor(Qt::red); // FIX ME - get a random color.

            DataTreeNode *node = new DataTreeNode();
            setNodeData(node, NODE_DATA_NAME,     name);
            setNodeData(node, NODE_DATA_FULLNAME, fullname);
            setNodeData(node, NODE_DATA_TYPE,     datasetType);
            setNodeData(node, NODE_DATA_COLOR,    color);
            setNodeData(node, NODE_DATA_SELECTED, false);
            setNodeData(node, NODE_DATA_ACTIVE,   false);
            setNodeData(node, NODE_DATA_VISIBILE, true);
            setNodeData(node, NODE_DATA_MASK,     false);

            QTreeWidgetItem * child = new QTreeWidgetItem(QStringList({name, "", ""}));

            setItemNode(child, node);

            child->setIcon(ITEM_NAME,  GetLightIcon(color));
            child->setIcon(ITEM_COLOR, GetColorIcon(color));

            parent->addChild(child);
            getItemNode(parent)->children.push_back(node);

            ExpandParent(child);

            mCurrentTreeWidgetItem = child;
            WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);

            emit updateViewWindow(mCurrentViewWindow);
        }
        else
        {
            QString message("Cannot add the dataset because the dataset type is not known.");
            emit postErrorMessage(message);
        }
    }
    else
    {
        QString message;
        if(datasetType == DatasetAttributes::Volume)
            message = tr("Cannot add the dataset because a volume group is not selected.");
        else if(datasetType == DatasetAttributes::Mesh)
            message = tr("Cannot add the dataset because a mesh group is not selected.");
        else
            message = tr("Cannot add the dataset because the dataset type is not known.");

        emit postErrorMessage(message);
    }
}

void QvisWorkspaceManager::WorkspaceDelete(const QString delName, DatasetAttributes::DatasetType datasetType, const QString fullname)
{
    // Traverse the workspace tree and delete the dataset name(s).
    // Note if a dataset is present multiple times the root name can be
    // deleted for ALL instances.
    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    bool showWarning = true;

    // If present, delete the dataset(s) from the tree.
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QTreeWidgetItem *treeItem = *it;
        it++;

        // Only delete datasets with the same type and fullname.
        DatasetAttributes::DatasetType dType = getItemType(treeItem);
        QString dFullname = getNodeData(treeItem, NODE_DATA_FULLNAME).toString();

        if(datasetType != dType || fullname != dFullname)
            continue;

        // Only delete datasets with the same base name.
        const QString name = treeItem->text(ITEM_NAME);

        // Delete the dataset with the same name.
        if(name == delName)
        {
        }
        // Warn the user that datasets with a different name but the same source will be deleted.
        else if(fullname == dFullname)
        {
            if(showWarning)
            {
                QMessageBox dialog;
                dialog.setText("Additional datasets have been found that have the same source as, '" + delName + ".' "
                               "All datasets will be deleted.");
                dialog.setInformativeText("This warning will not be repeated.");
                dialog.setStandardButtons(QMessageBox::Ok);
                dialog.exec();

                showWarning = false;
            }
        }
        // Name does not match.
        else
            continue;

        WorkspaceSelectionDeleteClicked(name);
    }
}

void QvisWorkspaceManager::WorkspaceSelected(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    mCurrentTreeWidgetItem = item;

    // Based on the item selected figure out what sibling items upstream and downstream would be current.
    mCurrentViewWindow  = tr("");
    mCurrentVolumeGroup = tr("");
    mCurrentVolume      = tr("");
    mCurrentMeshGroup   = tr("");
    mCurrentMesh        = tr("");

    if(mCurrentTreeWidgetItem == nullptr)
        return;

    QString text = mCurrentTreeWidgetItem->text(ITEM_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    // Based what item is selected set the up stream parents and down stream children
    // that can be considered to current (though not highlighted in the UI).
    if(dType == DatasetAttributes::ActiveDatasets)
    {
        // Only set the mCurrentViewWindow if there is one view
        if(mCurrentTreeWidgetItem->childCount() == 1)
        {
            QTreeWidgetItem *rvItem = mCurrentTreeWidgetItem->child(0);

            ViewWindowSelected(rvItem);
        }
    }
    else if(dType == DatasetAttributes::ViewWindow)
    {
        ViewWindowSelected(mCurrentTreeWidgetItem);
    }
    else if(dType == DatasetAttributes::VolumeGroup)
    {
        VolumeGroupSelected(mCurrentTreeWidgetItem);
    }
    else if(dType == DatasetAttributes::MeshGroup)
    {
        MeshGroupSelected(mCurrentTreeWidgetItem);
    }
    else
    {
        if(dType == DatasetAttributes::Volume)
            VolumeSelected(mCurrentTreeWidgetItem);
        else if(dType == DatasetAttributes::Mesh)
            MeshSelected(mCurrentTreeWidgetItem);

        // If a dataset was selected in the tree find it in the table
        // and select it too.
        emit datasetUpdateSelection(text, true);
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '"
              << text.toStdString() << "'  '"
              << mCurrentViewWindow.toStdString() << "'  '"
              << mCurrentVolumeGroup.toStdString() << "'  '"
              << mCurrentVolume.toStdString() << "'  '"
              << mCurrentMeshGroup.toStdString() << "'  '"
              << mCurrentMesh.toStdString() << "'  "
              << std::endl;

    WorkspaceUpdate(NODE_DATA_SELECTED, true);
}

void QvisWorkspaceManager::ViewWindowSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::ViewWindow)
    {
        if(mCurrentViewWindow != item->text(ITEM_NAME))
        {
            mCurrentViewWindow = item->text(ITEM_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        bool multpleVolumes = false;
        bool multpleMeshes = false;

        // Set the current volume and/or mesh group. But only if there is one of each.
        // Otherwise it is ambiguous.
        for(int i=0; i<item->childCount(); ++i)
        {
            QTreeWidgetItem *child = item->child(i);
            QString text = child->text(ITEM_NAME);
            DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

            if(dType == DatasetAttributes::VolumeGroup && !multpleVolumes)
            {
                // Only set the current volume group if there is one volume group
                if(mCurrentVolumeGroup == tr(""))
                {
                    mCurrentVolumeGroup = text;

                    // Only set the current volume if there is one volume
                    if(child->childCount() == 1)
                        mCurrentVolume = child->child(0)->text(ITEM_NAME);
                    else
                        mCurrentVolume = tr("");
                }
                // Found a second volume group so set to unknown.
                else
                {
                    mCurrentVolumeGroup = tr("");
                    mCurrentVolume = tr("");
                    multpleVolumes = true;
                }
            }
            else if(dType == DatasetAttributes::MeshGroup && !multpleMeshes)
            {
                // Only set the current mesh group if there is one mesh group
                if(mCurrentMeshGroup == tr(""))
                {
                    mCurrentMeshGroup = text;

                    // Only set the current mesh if there is one mesh
                    if(child->childCount() == 1)
                        mCurrentMesh = child->child(0)->text(ITEM_NAME);
                    else
                        mCurrentMesh = tr("");
                }
                // Found a second mesh group so set to unknown.
                else
                {
                    mCurrentMeshGroup = tr("");
                    mCurrentMesh = tr("");
                    multpleMeshes = true;
                }
            }
        }
    }

    if(mCurrentVolumeGroup.isEmpty())
        mCurrentVolumeGroup = mLastSelections[mCurrentViewWindow].volumeGroup;
    else
        mLastSelections[mCurrentViewWindow].volumeGroup = mCurrentVolumeGroup;

    if(mCurrentVolume.isEmpty())
        mCurrentVolume = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];
    else
        mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup] = mCurrentVolume;

    if(mCurrentMeshGroup.isEmpty())
        mCurrentMeshGroup = mLastSelections[mCurrentViewWindow].meshGroup;
    else
        mLastSelections[mCurrentViewWindow].meshGroup = mCurrentMeshGroup;

    if(mCurrentMesh.isEmpty())
        mCurrentMesh = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];
    else
        mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup] = mCurrentMesh;
}

void QvisWorkspaceManager::VolumeGroupSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::VolumeGroup)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(ITEM_NAME))
        {
            mCurrentViewWindow = item->parent()->text(ITEM_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentMeshGroup  = mLastSelections[mCurrentViewWindow].meshGroup;
        mCurrentMesh       = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];

        // Now set the volume group.
        mCurrentVolumeGroup = item->text(ITEM_NAME);
        mLastSelections[mCurrentViewWindow].volumeGroup = mCurrentVolumeGroup;

        // Check to see if the current volume is in the current volume group.
        mCurrentVolume = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];
        QTreeWidgetItem *volumeItem = GetTreeWidgetItem(mCurrentVolume);

        if(volumeItem == nullptr || volumeItem->parent()->text(ITEM_NAME) != mCurrentVolumeGroup)
        {
            // Only set the current volume if there is one volume
            if(item->childCount() == 1)
                mCurrentVolume = item->child(0)->text(ITEM_NAME);
            else
                mCurrentVolume = tr("");

            mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup] = mCurrentVolume;
        }
    }
}

void QvisWorkspaceManager::MeshGroupSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::MeshGroup)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(ITEM_NAME))
        {
            mCurrentViewWindow = item->parent()->text(ITEM_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentVolumeGroup = mLastSelections[mCurrentViewWindow].volumeGroup;
        mCurrentVolume      = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];

        // Now set the mesh group.
        mCurrentMeshGroup = item->text(ITEM_NAME);
        mLastSelections[mCurrentViewWindow].meshGroup = mCurrentMeshGroup;

        // Check to see if the current mesh is in the current mesh group.
        mCurrentMesh = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];
        QTreeWidgetItem *meshItem = GetTreeWidgetItem(mCurrentMesh);

        if(meshItem == nullptr || meshItem->parent()->text(ITEM_NAME) != mCurrentMeshGroup)
        {
            // Only set the current mesh if there is one mesh
            if(item->childCount() == 1)
                mCurrentMesh = item->child(0)->text(ITEM_NAME);
            else
                mCurrentMesh = tr("");

            mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup] = mCurrentMesh;
        }
    }
}

void QvisWorkspaceManager::VolumeSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::Volume)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(ITEM_NAME))
        {
            mCurrentViewWindow = item->parent()->parent()->text(ITEM_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentMeshGroup  = mLastSelections[mCurrentViewWindow].meshGroup;
        mCurrentMesh       = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];

        // Now set the volume group and volume.
        mCurrentVolumeGroup = item->parent()->text(ITEM_NAME);
        mCurrentVolume      = item->text(ITEM_NAME);

        emit activeViewWindowVolumeChanged(mCurrentViewWindow, mCurrentVolume);

        mLastSelections[mCurrentViewWindow].volumeGroup = mCurrentVolumeGroup;
        mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup] = mCurrentVolume;
    }
}

void QvisWorkspaceManager::MeshSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::Mesh)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(ITEM_NAME))
        {
            mCurrentViewWindow = item->parent()->parent()->text(ITEM_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentVolumeGroup = mLastSelections[mCurrentViewWindow].volumeGroup;
        mCurrentVolume      = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];

        // Now set the mesh group and mesh.
        mCurrentMeshGroup  = item->parent()->text(ITEM_NAME);
        mCurrentMesh       = item->text(ITEM_NAME);

        emit activeViewWindowMeshChanged(mCurrentViewWindow, mCurrentMesh);

        mLastSelections[mCurrentViewWindow].meshGroup = mCurrentMeshGroup;
        mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup] = mCurrentMesh;
    }
}

void QvisWorkspaceManager::WorkspaceIconClicked(QTreeWidgetItem *item, int column)
{
    if(item == mCurrentTreeWidgetItem && column == ITEM_NAME)
    {
        WorkspaceHideShowClicked();
    }
    else if(item == mCurrentTreeWidgetItem && column == ITEM_COLOR)
    {
        WorkspaceRandomizeColors();
    }
}

void QvisWorkspaceManager::WorkspaceDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    // Emit a signal to that will ALWAYS bring up the window.
    if(item == mCurrentTreeWidgetItem) // && column == ITEM_NAME)
    {
        if(dType == DatasetAttributes::ViewWindow)
            emit mViewWindowActionList[0]->triggered(true);
        else if(dType == DatasetAttributes::Volume)
            emit mVolumeActionList[0]->triggered(true);
        else if(dType == DatasetAttributes::Mesh)
            emit mMeshActionList[0]->triggered(true);
    }
}

void QvisWorkspaceManager::WorkspaceItemMoved(QTreeWidgetItem *item,      int iColumn,
                                              QTreeWidgetItem *oldParent, int pColumn)
{
    Q_UNUSED(iColumn);
    Q_UNUSED(pColumn);

    // When moving data the views need to be updated.
    if(DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt()) != DatasetAttributes::ViewWindow)
    {
        // Update the view that the data was moved to.
        QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(ITEM_NAME));
        emit updateViewWindow(viewItem->text(ITEM_NAME));

        // Update the view that the data was moved from.
        viewItem = GetTreeWidgetItemViewWindow(oldParent->text(ITEM_NAME));
        emit updateViewWindow(viewItem->text(ITEM_NAME));
    }

    DataTreeNode * node = getItemNode(item);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << node->name << std::endl;

    // Removed the node from the old parent.
    getItemNode(oldParent)->children.remove(node);

    // Get the new parent item and node.
    QTreeWidgetItem *newParent = item->parent();
    DataTreeNode * newParentNode = getItemNode(newParent);

    // Advance the iterator to match the index.
    size_t index = newParent->indexOfChild(item);
    std::list<DataTreeNode *>::iterator iter = newParentNode->children.begin();
    for (size_t i=0; i<index; ++i, ++iter);

    // Insert the child.
    newParentNode->children.insert(iter, node);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '"
              << item->text(0).toStdString()
              << "'  new parent '"
              << item->parent()->text(0).toStdString() << "'  " << index
              << "  old parent '"
              << oldParent->text(0).toStdString() << "'  " << std::endl;
}

void QvisWorkspaceManager::WorkspaceChanged(QTreeWidgetItem *item, int column)
{
    if(ITEM_NAME != column)
        return;

    // Only volume and mesh groups can be renamed.
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());
    if((dType & (DatasetAttributes::VolumeGroup | DatasetAttributes::MeshGroup)) == 0)
        return;

    QString newName = item->text(ITEM_NAME);

    QString tmpName = newName.simplified(); // New name with whitespace cleanup.
    tmpName.remove(" "); // Remove all whitespace;

    // Check to make sure the name is unique.
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QString tmp = (*it)->text(ITEM_NAME).simplified();
        tmp.remove(" "); // Remove all whitespace;

        if((*it) != item && tmpName == tmp)
        {
            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

            if(dType == DatasetAttributes::VolumeGroup)
                item->setText(ITEM_NAME, mCurrentVolumeGroup);
            else if(dType == DatasetAttributes::MeshGroup)
                item->setText(ITEM_NAME, mCurrentMeshGroup);

            QString message("Cannot rename the group because the name is already in use.");
            emit postErrorMessage(message);

            return;
        }

        ++it;
    }

    // Update the last selection map with the new volume / mesh group name.
    std::map<QString, QString>::node_type nodeHandler;
    if(dType == DatasetAttributes::VolumeGroup)
        nodeHandler = mLastSelections[mCurrentViewWindow].datasets.extract(mCurrentVolumeGroup);
    else if(dType == DatasetAttributes::MeshGroup)
        nodeHandler = mLastSelections[mCurrentViewWindow].datasets.extract(mCurrentMeshGroup);

    nodeHandler.key() = newName;
    mLastSelections[mCurrentViewWindow].datasets.insert(std::move(nodeHandler));

    WorkspaceSelected(item, ITEM_NAME);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << item->text(ITEM_NAME).toStdString() << "'  " << dType << std::endl;
}

void QvisWorkspaceManager::ExpandParent(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();

    if(parent)
    {
        parent->setExpanded(true);
        ExpandParent(parent);
    }
}

QTreeWidgetItem * QvisWorkspaceManager::GetTreeWidgetItem(const QString &name, bool exact) const
{
    if(name.isEmpty())
        return nullptr;

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QString tmp = (*it)->text(ITEM_NAME);

        if((exact && tmp == name) || (!exact && tmp.startsWith(name)))
            return (*it);

        ++it;
    }

    return nullptr;
}

QTreeWidgetItem * QvisWorkspaceManager::GetTreeWidgetItemViewWindow(const QString &name) const
{
    if(name.isEmpty())
        return nullptr;

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);
    QTreeWidgetItem *view = nullptr;

    while(*it)
    {
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType((*it)->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

        if(dType == DatasetAttributes::ViewWindow)
            view = *it;

        QString tmp = (*it)->text(ITEM_NAME);

        if(tmp == name)
            return (view);

        ++it;
    }

    return nullptr;
}

QMenu * QvisWorkspaceManager::GetTreeWidgetMenu(QMenu *parent, const DatasetAttributes::DatasetType datasetType)
{
    QMenu *menu = new QMenu(parent);
    menu->setTitle("Add to");

    // Use a QSignalMapper which will map the view window name and volume/mesh group name to the DatasetAddClicked slot.
    QSignalMapper* signalMapper = new QSignalMapper (parent) ;
    QObject::connect(signalMapper, &QSignalMapper::mappedString, mDatasetManager, &QvisDatasetManager::DatasetAddClicked);

    for(int i=0; i<ui->WorkspaceTreeWidget->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem * item = ui->WorkspaceTreeWidget->topLevelItem(i);

        GetTreeWidgetMenu(datasetType, item, menu, signalMapper);
    }

    return menu;
}

void QvisWorkspaceManager::GetTreeWidgetMenu(const DatasetAttributes::DatasetType datasetType, const QTreeWidgetItem *item, QMenu *menu, QSignalMapper* signalMapper) const
{
    QString text = item->text(ITEM_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::ActiveDatasets)
    {
        for(int i=0; i<item->childCount(); ++i)
        {
            const QTreeWidgetItem *child = item->child(i);
            GetTreeWidgetMenu(datasetType, child, menu, signalMapper);
        }
    }
    else if(dType == DatasetAttributes::ViewWindow)
    {
        int count = 0;

        // Check for volume and mesh groups.
        for(int i=0; i<item->childCount(); ++i)
        {
            const QTreeWidgetItem *child = item->child(i);
            DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

            if((datasetType == DatasetAttributes::Volume && dType == DatasetAttributes::VolumeGroup) ||
               (datasetType == DatasetAttributes::Mesh && dType == DatasetAttributes::MeshGroup))
                ++count;
        }

        if(count == 0)
        {
            // If no groups then just an entry for the view window.
            QAction *action = menu->addAction(text);
            signalMapper->setMapping(action, text); // Just the view window name
            connect (action, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
        }
        else
        {
            // If one or more groups so create a sub menu
            QMenu *childMenu = new QMenu(menu);
            childMenu->setTitle(text);
            menu->addMenu(childMenu);

            for(int i=0; i<item->childCount(); ++i)
            {
                const QTreeWidgetItem *child = item->child(i);
                GetTreeWidgetMenu(datasetType, child, childMenu, signalMapper);
            }
        }
    }
    else if((datasetType == DatasetAttributes::Volume && dType == DatasetAttributes::VolumeGroup) ||
            (datasetType == DatasetAttributes::Mesh   && dType == DatasetAttributes::MeshGroup))
    {
        QAction *action = menu->addAction(text);
        signalMapper->setMapping(action, text); // Just the mesh group name
        connect (action, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    }
}

void QvisWorkspaceManager::WorkspaceContextMenuRequested(const QPoint &pos)
{
    mCurrentTreeWidgetItem = ui->WorkspaceTreeWidget->itemAt(pos);

    if(mCurrentTreeWidgetItem == nullptr)
        return;

    WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);

    QString text = mCurrentTreeWidgetItem->text(ITEM_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());
    bool visible = getNodeData(mCurrentTreeWidgetItem, NODE_DATA_VISIBILE).toBool();
    QString visibleStr = tr("Toggle Visibility") + (visible ? tr(" - Hide") : tr(" - Show"));

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << text.toStdString() << "'  " << dType << std::endl;

    // Create a new context menu.
    QMenu *contextMenu = new QMenu(this);

    // Collapse / expand for parent items
    if(mCurrentTreeWidgetItem->childCount())
    {
        QString toggleState;
        if(mCurrentTreeWidgetItem->isExpanded())
            toggleState = tr("Collapse");
        else
            toggleState = tr("Expand");

        contextMenu->addAction(toggleState, this, SLOT(WorkspaceToggleTreeWidgetItem()));

        contextMenu->addSeparator();
    }

    // For each data type add in specifc menu items.
    if(dType == DatasetAttributes::ActiveDatasets)
    {
        contextMenu->addAction("Add View Window", this, SLOT(WorkspaceViewWindowAddClicked()));
    }
    else if(dType == DatasetAttributes::ViewWindow)
    {
        if(mCurrentViewWindow != text)
        {
            mCurrentViewWindow = text;
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        bool foundGrandChildren = false;

        for(int i=0; i<mCurrentTreeWidgetItem->childCount(); ++i)
        {
            if(mCurrentTreeWidgetItem->child(i)->childCount())
            {
                foundGrandChildren = true;
                break;
            }
        }

        if(foundGrandChildren)
        {
            contextMenu->addAction(visibleStr,         this, SLOT(WorkspaceHideShowClicked()));
            contextMenu->addAction("Show All",         this, SLOT(WorkspaceShowAll()));
            contextMenu->addAction("Randomize Colors", this, SLOT(WorkspaceRandomizeColors()));
            contextMenu->addSeparator();
        }

        contextMenu->addAction("Add Volume Group", this, [=]() {WorkspaceGroupAddClicked(DatasetAttributes::VolumeGroup);});
        contextMenu->addAction("Add Mesh Group",   this, [=]() {WorkspaceGroupAddClicked(DatasetAttributes::MeshGroup);});

        if(ui->WorkspaceTreeWidget->topLevelItem(0)->childCount() > 1)
        {
            contextMenu->addSeparator();

            // TO DO use the toggle view action from the actual view window
            if(0)
                contextMenu->addAction("Hide/Show",  this, SLOT(ViewWindowToggleView()));
            else
                contextMenu->addAction("Hide",  this, SLOT(ViewWindowToggleView()));

            contextMenu->addAction("Delete", this, SLOT(WorkspaceSelectionDeleteClicked()));
        }

        // Add actions that toggle dialogs that are relevant to view windows
        if(mViewWindowActionList.size())
        {
            contextMenu->addSeparator();

            for(auto *action : mViewWindowActionList)
                contextMenu->addAction(action);
        }
    }
    else if(dType & (DatasetAttributes::VolumeGroup | DatasetAttributes::MeshGroup))
    {
        if(mCurrentTreeWidgetItem->childCount())
        {
            contextMenu->addAction(visibleStr,         this, SLOT(WorkspaceHideShowClicked()));
            contextMenu->addAction("Show All",         this, SLOT(WorkspaceShowAll()));
            contextMenu->addAction("Isolate",          this, SLOT(WorkspaceIsolate()));
            contextMenu->addAction("Randomize Colors", this, SLOT(WorkspaceRandomizeColors()));
            contextMenu->addSeparator();
        }

        contextMenu->addAction("Rename", this, SLOT(WorkspaceRename()));
        contextMenu->addAction("Delete", this, SLOT(WorkspaceSelectionDeleteClicked()));
    }
    else if(dType & (DatasetAttributes::Volume | DatasetAttributes::Mesh))
    {
        contextMenu->addAction(visibleStr,         this, SLOT(WorkspaceHideShowClicked()));
        contextMenu->addAction("Isolate",          this, SLOT(WorkspaceIsolate()));
        contextMenu->addAction("Randomize Colors", this, SLOT(WorkspaceRandomizeColors()));
        contextMenu->addSeparator();
        contextMenu->addAction("Delete",           this, SLOT(WorkspaceSelectionDeleteClicked()));

        if(dType == DatasetAttributes::Volume)
        {
            contextMenu->addSeparator();
            contextMenu->addAction("Copy Mask",         this, SLOT(VolumeCopyMask()));

            bool haveMask = true; // FIX ME

            if(haveMask)
            {
                contextMenu->addAction("Paste Mask",     this, SLOT(VolumePasteMask()));
                contextMenu->addAction("Merge Mask",     this, SLOT(VolumeMergeMask()));
                contextMenu->addAction("Exclude Mask",   this, SLOT(VolumeExcludeMask()));
                contextMenu->addAction("Intersect Mask", this, SLOT(VolumeIntersectMask()));
            }

            // Add actions that toggle dialogs that are relevant to volumes
            if(mVolumeActionList.size())
            {
                contextMenu->addSeparator();

                for(auto *action : mVolumeActionList)
                    contextMenu->addAction(action);
            }
        }
        else if(dType == DatasetAttributes::Mesh)
        {
            // Add actions that toggle dialogs that are relevant to meshes
            if(mMeshActionList.size())
            {
                contextMenu->addSeparator();

                for(auto *action : mMeshActionList)
                    contextMenu->addAction(action);
            }
        }
    }

    contextMenu->popup(ui->WorkspaceTreeWidget->viewport()->mapToGlobal(pos));
}

void QvisWorkspaceManager::WorkspaceToggleTreeWidgetItem()
{
    mCurrentTreeWidgetItem->setExpanded(!mCurrentTreeWidgetItem->isExpanded());
}

void QvisWorkspaceManager::WorkspaceShowAll(QTreeWidgetItem *item)
{
    // The initial call is from the menu thus item is null so use the current item.
    // Subsuquent recursive calls the item should be valid.
    if(item == nullptr)
        item = mCurrentTreeWidgetItem;

    if(item == nullptr)
        return;

    // Make the current item visible.
    QColor color = getNodeData(item, NODE_DATA_COLOR).value<QColor>();

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << color.value() << std::endl;

    item->setIcon(ITEM_NAME, GetLightIcon(color));
    setNodeData(mCurrentTreeWidgetItem, NODE_DATA_VISIBILE, true);

    // Make all of the children visible.
    for(int i=0; i<item->childCount(); i++)
    {
        WorkspaceShowAll(item->child(i));
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(ITEM_NAME));
    emit updateViewWindow(viewItem->text(ITEM_NAME));
}

void QvisWorkspaceManager::WorkspaceIsolate()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    QTreeWidgetItem *parent = mCurrentTreeWidgetItem->parent();

    if(parent == nullptr)
        return;

    // Make all of the children hidden except the current, make it visible.
    for(int i=0; i<parent->childCount(); i++)
    {
        QTreeWidgetItem *child = parent->child(i);
        bool visible = getNodeData(child, NODE_DATA_VISIBILE).toBool();

        if(child == mCurrentTreeWidgetItem)
        {
            if(!visible)
            {
                QColor color = getNodeData(child, NODE_DATA_COLOR).value<QColor>();

                child->setIcon(ITEM_NAME, GetLightIcon(color));
                setNodeData(child, NODE_DATA_VISIBILE, true);
            }
        }
        else if(visible)
        {
            child->setIcon(ITEM_NAME, GetLightIcon(QColor(Qt::black)));
            setNodeData(child, NODE_DATA_VISIBILE, false);
        }
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(parent->text(ITEM_NAME));
    emit updateViewWindow(viewItem->text(ITEM_NAME));
}

void QvisWorkspaceManager::WorkspaceRandomizeColors(QTreeWidgetItem *item)
{
    // The initial call is from the menu thus item is null so use the current item.
    // Subsuquent recursive calls the item should be valid.
    if(item == nullptr)
        item = mCurrentTreeWidgetItem;

    if(item == nullptr)
        return;

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(item->childCount())
    {
        for(int i=0; i<item->childCount(); i++)
        {
            WorkspaceRandomizeColors(item->child(i));
        }
    }
    else if(dType & (DatasetAttributes::Volume | DatasetAttributes::Mesh))
    {
        QColor color(Qt::green); // FIX ME - get a random color.
        setNodeData(item, NODE_DATA_COLOR, color);

        bool visible = getNodeData(item, NODE_DATA_VISIBILE).toBool();

        if(visible)
            item->setIcon(ITEM_NAME, GetLightIcon(color));
        else
            item->setIcon(ITEM_NAME, GetLightIcon(QColor(Qt::black)));

        item->setIcon(ITEM_COLOR, GetColorIcon(color));
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(ITEM_NAME));
    emit updateViewWindow(viewItem->text(ITEM_NAME));
}

void QvisWorkspaceManager::WorkspaceRename()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    ui->WorkspaceTreeWidget->editItem(mCurrentTreeWidgetItem);
}

void QvisWorkspaceManager::ViewWindowToggleView()
{
    emit actionVewWindowToggleView();
}

void QvisWorkspaceManager::VolumeCopyMask()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    QString text = mCurrentTreeWidgetItem->text(ITEM_NAME);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << text.toStdString() << std::endl;

    WorkspaceUpdate(NODE_DATA_MASK, true);
}

void QvisWorkspaceManager::VolumePasteMask()
{

}

void QvisWorkspaceManager::VolumeMergeMask()
{

}

void QvisWorkspaceManager::VolumeExcludeMask()
{

}

void QvisWorkspaceManager::VolumeIntersectMask()
{

}

// Workspace buttons
void QvisWorkspaceManager::WorkspaceHideShowClicked()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dType != DatasetAttributes::ActiveDatasets)
    {
        // Flip the visibility.
        bool visible = !getNodeData(mCurrentTreeWidgetItem, NODE_DATA_VISIBILE).toBool();

        setNodeData(mCurrentTreeWidgetItem, NODE_DATA_VISIBILE, visible);

        QColor color;

        if(visible)
            color = getNodeData(mCurrentTreeWidgetItem, NODE_DATA_COLOR).value<QColor>();
        else
            color = QColor(Qt::black);

        mCurrentTreeWidgetItem->setIcon(ITEM_NAME, GetLightIcon(color));

        QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(mCurrentTreeWidgetItem->text(ITEM_NAME));
        emit updateViewWindow(viewItem->text(ITEM_NAME));
    }
}

void QvisWorkspaceManager::WorkspaceSelectionDeleteClicked(QString name)
{
    // If the name is present it is call from the main window so to
    // force a view window to be deleted.
    if(!name.isEmpty())
    {
        mCurrentTreeWidgetItem = GetTreeWidgetItem(name);

        // If the item was not found that means the data manager deleted the view then signaled
        // the main window (see the emit actionViewWindowDelete below). When the main window deletes
        // a view it calls this method. As view was already deleted there is nothing to be done.
        if(mCurrentTreeWidgetItem == nullptr)
            return;
    }

    if(mCurrentTreeWidgetItem == nullptr)
    {
        QString message("Cannot delete, no current selection.");
        emit postErrorMessage(message);
        return;
    }

    DatasetAttributes::DatasetType dataType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

    if(dataType == DatasetAttributes::ActiveDatasets)
    {
        QString message("Cannot delete the Active Datasets node.");
        emit postErrorMessage(message);
        return;
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << "  "
              << mCurrentTreeWidgetItem->text(ITEM_NAME).toStdString() << "  " << std::endl;

    QTreeWidgetItem *parent = mCurrentTreeWidgetItem->parent();

    if(parent)
    {
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(parent->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());

        std::cerr << __FUNCTION__ << "  " << __LINE__ << " parent " << parent->text(0).toStdString() << "  " << parent->childCount() << "  "
                  << dType << "  " << std::endl;

        // If the name is empty then there must be at least one view window. Otherwise
        // the main window is forcing a view window to be deleted.
        int minChildren = name.isEmpty() ? 1 : 0;
        if((dType == DatasetAttributes::ActiveDatasets && parent->childCount() > minChildren) ||
           (dType != DatasetAttributes::ActiveDatasets))
        {
            // Before deleting the item get its view, sibling, and number of child so the view can be updated if needed.
            QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(mCurrentTreeWidgetItem->text(ITEM_NAME));
            int nChildren = mCurrentTreeWidgetItem->childCount();

            int sibling = std::min(parent->indexOfChild(mCurrentTreeWidgetItem), parent->childCount()-1);

            delete getItemNode(mCurrentTreeWidgetItem);

            parent->removeChild(mCurrentTreeWidgetItem);

            if(dataType == DatasetAttributes::ViewWindow)
            {
                // If the name is empty then the signal was from the data manager so signal the main window.
                // If the name is not empty then the signal was from the main window.
                if(name.isEmpty())
                    emit actionViewWindowDelete(name);
            }
            else if((dataType & (DatasetAttributes::VolumeGroup | DatasetAttributes::MeshGroup) && nChildren) ||
                    (dataType & (DatasetAttributes::Volume      | DatasetAttributes::Mesh)))
            {
                emit updateViewWindow(viewItem->text(ITEM_NAME));
            }

            // After deleting the item, the newly selected item will be a sibbling or the parent so
            // set the mCurrentTreeWidgetItem
            if(parent->childCount() == 0)
                mCurrentTreeWidgetItem = parent;
            else
                mCurrentTreeWidgetItem = parent->child(sibling);

            WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);
        }
        else if(dType == DatasetAttributes::ActiveDatasets && parent->childCount() == 1)
        {
            QString message("Cannot delete the view window, there must be at least one view window.");
            emit postErrorMessage(message);
        }
    }
    else
    {
        QString message("Cannot delete, no parent. Malformed tree.");
        emit postErrorMessage(message);
    }
}

void QvisWorkspaceManager::WorkspaceViewWindowSelected(QString name)
{
    if(name != mCurrentViewWindow)
    {
        mCurrentTreeWidgetItem = GetTreeWidgetItem(name);

        // Select the current item in the tree view.
        WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);

        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << name.toStdString() << std::endl;

        // Select the current item in the dataset list.
        emit datasetUpdateSelection(mCurrentMesh,   true);
        emit datasetUpdateSelection(mCurrentVolume, true);
/* FIXME
        // Volume item already selected - nothing to do
        QTableWidgetItem *volumeTableItem = GetTableWidgetItem(mCurrentVolume);
        if(volumeTableItem && mCurrentDataSetIndex == volumeTableItem->row())
            return;

        // Mesh item already selected - nothing to do
        QTableWidgetItem *meshTableItem = GetTableWidgetItem(mCurrentMesh);
        if(meshTableItem && mCurrentDataSetIndex == meshTableItem->row())
            return;

        // Volume item not selected - so select it
        if(volumeTableItem)
            mCurrentDataSetIndex = volumeTableItem->row();
        // Mesh item not selected - so select it
        else if(meshTableItem)
            mCurrentDataSetIndex = meshTableItem->row();
        else
            mCurrentDataSetIndex = -1;

        // Only allow one dataset to be selected.
        DatasetUpdate(mCurrentDataSetIndex, true);
FIXME */
    }
}

void QvisWorkspaceManager::WorkspaceViewWindowRename(QString currentName, QString newName)
{
    QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    QTreeWidgetItem *widget = GetTreeWidgetItem(currentName);

    widget->setText(ITEM_NAME, newName);
}

QTreeWidgetItem * QvisWorkspaceManager::WorkspaceActiveDatasetsAdd(DataTreeNode *node)
{
    // Create an initial node for the tree
    if(node == nullptr)
    {
        node = new DataTreeNode();
        setNodeData(node, NODE_DATA_NAME,     ACTIVE_DATASETS_STR);
        setNodeData(node, NODE_DATA_TYPE,     DatasetAttributes::ActiveDatasets);
        setNodeData(node, NODE_DATA_COLOR,    QColor(Qt::white));
        setNodeData(node, NODE_DATA_SELECTED, false);
        setNodeData(node, NODE_DATA_ACTIVE,   false);
        setNodeData(node, NODE_DATA_VISIBILE, true);
        setNodeData(node, NODE_DATA_MASK,     false);

        mAgent->setDataTree(node);
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList({ACTIVE_DATASETS_STR}));

    setItemNode(item, node);

    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);

    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);
    ui->WorkspaceTreeWidget->addTopLevelItem(item);

    return item;
}

void QvisWorkspaceManager::WorkspaceViewWindowAddClicked(QString name)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << std::endl;

    // If the name is empty it is a local slot call and the main window handles adding view windows.
    if(name.isEmpty())
    {
        emit actionViewWindowAdd();
    }
    // If the name is present then communication from the main window to update the tree.
    else
    {
        QTreeWidgetItem *parent = GetTreeWidgetItem(ACTIVE_DATASETS_STR);

        if(parent)
        {
            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

            DataTreeNode *node = new DataTreeNode();
            setNodeData(node, NODE_DATA_NAME,     name);
            setNodeData(node, NODE_DATA_TYPE,     DatasetAttributes::ViewWindow);
            setNodeData(node, NODE_DATA_COLOR,    QColor(Qt::white));
            setNodeData(node, NODE_DATA_SELECTED, false);
            setNodeData(node, NODE_DATA_ACTIVE,   false);
            setNodeData(node, NODE_DATA_VISIBILE, true);
            setNodeData(node, NODE_DATA_MASK,     false);

            QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({name}));

            setItemNode(child, node);

            child->setIcon(ITEM_NAME,  GetLightIcon(QColor(Qt::white)));
            child->setIcon(ITEM_COLOR, GetColorIcon(QColor(Qt::white)));

            parent->addChild(child);
            getItemNode(parent)->children.push_back(node);

            ExpandParent(child);

            mCurrentTreeWidgetItem = child;
            WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);
        }
    }
}

QTreeWidgetItem * QvisWorkspaceManager::WorkspaceGroupAddClicked(DatasetAttributes::DatasetType dType)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentViewWindow.toStdString() << std::endl;

    QTreeWidgetItem *parent = GetTreeWidgetItem(mCurrentViewWindow);

    if(parent)
    {
        const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

        QString name;

        if(dType == DatasetAttributes::VolumeGroup)
            name = VOLUME_GROUP_STR + QString("%1").arg(mNumVolumeGroups++);
        else // if(dType == DatasetAttributes::MeshGroup)
            name = MESH_GROUP_STR + QString("%1").arg(mNumMeshGroups++);

        DataTreeNode *node = new DataTreeNode();
        setNodeData(node, NODE_DATA_NAME,     name);
        setNodeData(node, NODE_DATA_TYPE,     dType);
        setNodeData(node, NODE_DATA_COLOR,    QColor(Qt::white));
        setNodeData(node, NODE_DATA_SELECTED, false);
        setNodeData(node, NODE_DATA_ACTIVE,   false);
        setNodeData(node, NODE_DATA_VISIBILE, true);
        setNodeData(node, NODE_DATA_MASK,     false);

        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({name, "", ""}));
        child->setFlags(child->flags() | Qt::ItemIsEditable);

        setItemNode(child, node);

        child->setIcon(ITEM_NAME,  GetLightIcon(QColor(Qt::white)));
        child->setIcon(ITEM_COLOR, GetColorIcon(QColor(Qt::white)));

        parent->addChild(child);
        getItemNode(parent)->children.push_back(node);

        ExpandParent(child);

        mCurrentTreeWidgetItem = child;
        WorkspaceSelected(mCurrentTreeWidgetItem, ITEM_NAME);

        return child;
    }
    else
    {
        QString message("Cannot add a group, no view window is selected.");
        emit postErrorMessage(message);

        return nullptr;
    }
}

void QvisWorkspaceManager::WorkspaceUpdate(NodeDataIndex index, bool value)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << index << std::endl;

    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    // Set the values so at most ONE is selected or is the mask.
    while(*it)
    {
        bool val = getNodeData((*it), index).toBool();

        // If values are different, update the current item.
        if((*it) == mCurrentTreeWidgetItem)
        {
            if(val != value)
                setNodeData((*it), index, value);

            // Selected items are in bold
            QFont font = (*it)->font(ITEM_NAME);
            font.setWeight(value ? QFont::Bold : QFont::Normal);
            (*it)->setFont(ITEM_NAME, font);

            if(index == NODE_DATA_SELECTED)
            {
                if((*it)->isSelected() != value)
                {
                    (*it)->setSelected(value);

                    setNodeData((*it), NODE_DATA_ACTIVE, value);
                }
            }
            else if(index == NODE_DATA_MASK)
            {
                if(value)
                    (*it)->setIcon(ITEM_STATUS, QIcon(":Src/FluoRender/UI/icons/misc/Mask.png"));
                else
                    (*it)->setIcon(ITEM_STATUS, QIcon());
            }
        }
        // If values are different, update the other current items.
        else
        {
            bool active = ((*it)->text(ITEM_NAME) == mCurrentViewWindow ||
                           (*it)->text(ITEM_NAME) == mCurrentVolumeGroup ||
                           (*it)->text(ITEM_NAME) == mCurrentVolume ||
                           (*it)->text(ITEM_NAME) == mCurrentMeshGroup ||
                           (*it)->text(ITEM_NAME) == mCurrentMesh);

            if(val != false)
                setNodeData((*it), index, false);

            // Active items are in bold
            QFont font = (*it)->font(ITEM_NAME);
            font.setWeight(active ? QFont::Bold : QFont::Normal);
            (*it)->setFont(ITEM_NAME, font);

            if(index == NODE_DATA_SELECTED)
            {
                if((*it)->isSelected())
                    (*it)->setSelected(false);

                setNodeData((*it), NODE_DATA_ACTIVE, active);
            }
            else if(index == NODE_DATA_MASK)
            {
                (*it)->setIcon(ITEM_STATUS, QIcon());
            }
        }

        ++it;
    }
}

QString QvisWorkspaceManager::WorkspaceExists(QString name, DatasetAttributes::DatasetType dType)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << std::endl;

    int count = 0;
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    // Loop through all datasets of the same type and get the biggest index "_N" that might be
    // attached to the name.
    while(*it)
    {
        // Is it possible for a volume and mesh to have the same name???
        //if(DatasetAttributes::DatasetType((*it)->data(ITEM_DATA_TYPE, Qt::UserRole).toInt()) == dType)
        {
            QString tmp = (*it)->text(ITEM_NAME);

            if(tmp == name)
                count = std::max(count, 1);
            else if(tmp.startsWith(name))
                count = std::max(count, tmp.last(tmp.size()-name.size()-1).toInt() + 1);
        }

        ++it;
    }

    // If mulitple names append a number to the end.
    if(count)
        name += QString("_%1").arg(count);

    return name;
}

QIcon QvisWorkspaceManager::GetLightIcon(QColor color)
{
    // Get the base light - the interior of the light is transparent.
    QImage image(":Src/FluoRender/UI/icons/misc/LightBase.png");

    // Create pixmap of the same size and set the fill color which will be
    // the color of the interior of the light.
    QPixmap pixmap(image.size());
    pixmap.fill(color);

    // Create a painter with pximap as the base and draw the light image over it. The
    // painter is in braces as otherwise the mask cannot be set if the painter is active.
    {
        QPainter painter(&pixmap);
        painter.drawImage(0, 0, image);
    }

    // Now make the background transparent via a mask.
    QPixmap background(":Src/FluoRender/UI/icons/misc/LightBackground.png");
    pixmap.setMask(background.createMaskFromColor(Qt::transparent));

    return QIcon(pixmap);
}

QIcon QvisWorkspaceManager::GetColorIcon(QColor color)
{
    // Get the base circle - the interior of the circle is transparent.
    QImage image(":Src/FluoRender/UI/icons/misc/CircleBase.png");

    // Create pixmap of the same size and set the fill color which will be
    // the color of the interior of the light.
    QPixmap pixmap(image.size());
    pixmap.fill(color);

    // Create a painter with pximap as the base and draw the circle image over it. The
    // painter is in braces as otherwise the mask cannot be set if the painter is active.
    {
        QPainter painter(&pixmap);
        painter.drawImage(0, 0, image);
    }

    // Now make the background transparent via a mask.
    QPixmap background(":Src/FluoRender/UI/icons/misc/CircleBackground.png");
    pixmap.setMask(background.createMaskFromColor(Qt::transparent));

    return QIcon(pixmap);
}

void QvisWorkspaceManager::setItemData(QTreeWidgetItem *item, ItemDataIndex index, QVariant value)
{
    item->setData(index, Qt::UserRole, value);
}

QVariant QvisWorkspaceManager::getItemData(QTreeWidgetItem *item, ItemDataIndex index)
{
    return item->data(index, Qt::UserRole);
}

inline void QvisWorkspaceManager::setItemNode(QTreeWidgetItem *item, const DataTreeNode * node)
{
    // Save the pointer as qlonglong which allows it to work with the drag and drop.
    QVariant nodeVar;
    nodeVar.setValue((qlonglong)node);

    item->setData(ITEM_DATA_NODE, Qt::UserRole, nodeVar);
    item->setData(ITEM_DATA_TYPE, Qt::UserRole, node->type);
}

inline DataTreeNode * QvisWorkspaceManager::getItemNode(QTreeWidgetItem *item)
{
    return (DataTreeNode *) getItemData(item, ITEM_DATA_NODE).value<qlonglong>();
}

inline void QvisWorkspaceManager::setItemType(QTreeWidgetItem *item, const DatasetAttributes::DatasetType dType)
{
    item->setData(ITEM_DATA_TYPE, Qt::UserRole, dType);
}

inline DatasetAttributes::DatasetType QvisWorkspaceManager::getItemType(QTreeWidgetItem *item)
{
    return DatasetAttributes::DatasetType(item->data(ITEM_DATA_TYPE, Qt::UserRole).toInt());
}

inline void QvisWorkspaceManager::setNodeData(QTreeWidgetItem *item, NodeDataIndex index, QVariant value)
{
    DataTreeNode *node = getItemNode(item);

    setNodeData(node, index, value);
}

inline QVariant QvisWorkspaceManager::getNodeData(QTreeWidgetItem *item, NodeDataIndex index)
{
    DataTreeNode *node = getItemNode(item);

    return getNodeData(node, index);
}

inline void QvisWorkspaceManager::setNodeData(DataTreeNode *node, NodeDataIndex index, QVariant value)
{
    switch(index)
    {
    case NODE_DATA_NAME:
        node->name = value.toString().toStdString();
        break;
    case NODE_DATA_FULLNAME:
        node->fullname = value.toString().toStdString();
        break;
    case NODE_DATA_TYPE:
        node->type = DatasetAttributes::DatasetType(value.toInt());
        break;
    case NODE_DATA_COLOR:
    {
        QColor color = value.value<QColor>();

        node->color[0] = color.red();
        node->color[1] = color.green();
        node->color[2] = color.blue();
    }
        break;
    case NODE_DATA_SELECTED:
        node->selected = value.toBool();
        break;
    case NODE_DATA_ACTIVE:
        node->active = value.toBool();
        break;
    case NODE_DATA_VISIBILE:
        node->visible = value.toBool();
        break;
    case NODE_DATA_MASK:
        node->mask = value.toBool();
        break;
    }
}

QVariant QvisWorkspaceManager::getNodeData(DataTreeNode *node, NodeDataIndex index)
{
    switch(index)
    {
    case NODE_DATA_NAME:
        return QString(node->name.c_str());
        break;
    case NODE_DATA_FULLNAME:
        return QString(node->fullname.c_str());
        break;
    case NODE_DATA_TYPE:
        return node->type;
        break;
    case NODE_DATA_COLOR:
        return QColor(node->color[0], node->color[1], node->color[2]);
        break;
    case NODE_DATA_SELECTED:
        return node->selected;
        break;
    case NODE_DATA_ACTIVE:
        return node->active;
        break;
    case NODE_DATA_VISIBILE:
        return node->visible;
        break;
    case NODE_DATA_MASK:
        return node->mask;
        break;
    }
}

QList<QAction *> & QvisWorkspaceManager::getViewWindowActionList()
{
    return mViewWindowActionList;
}

QList<QAction *> & QvisWorkspaceManager::getVolumeActionList()
{
    return mVolumeActionList;
}

QList<QAction *> & QvisWorkspaceManager::getMeshActionList()
{
    return mMeshActionList;
}
