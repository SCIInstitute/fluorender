#include "QvisDataManager.h"
#include "ui_QvisDataManager.h"

#include "QtCore/qsize.h"
#include "QtCore/qpoint.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <QSplitter>
#include <QThread>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>

#include <QvisVCRWidget.h>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>

// Used with the dataset table widget which has multiple columns;
#define DATASET_TYPE 0
#define DATASET_NAME 1
#define DATASET_PATH 2

// Used with the workspace tree widget which has multiple columns;
// name and a status icon.
#define WORKSPACE_NAME   0
#define WORKSPACE_COLOR  1 // Note matches user data index below
#define WORKSPACE_STATUS 2

// Used with the QTreeWidgetItem for the user data in each item
#define WORKSPACE_TYPE     0
#define WORKSPACE_COLOR    1 // Note matches column index above
#define WORKSPACE_SELECTED 2
#define WORKSPACE_ACTIVE   3
#define WORKSPACE_VISIBILE 4
#define WORKSPACE_MASK     5

// Base names for the tree view groups.
#define ACTIVE_DATASETS QString("Active Datasets")
#define VIEW_WINDOW     QString("View Window ")
#define VOLUME_GROUP    QString("Volume Group ")
#define MESH_GROUP      QString("Mesh Group ")

// QvisAnimationWorker is a threaded worker for animating through a dataset.
// It is threaded so that the UI can continue to be active. Namely the user
// needs to be able to stop the animation.
QvisAnimationWorker::QvisAnimationWorker()
{
}

void QvisAnimationWorker::process()
{
    // DO TO - add the needed code for signalling the next frame.

    // The animation can be quick so add a pause between frames.
    std::this_thread::sleep_for(std::chrono::milliseconds(mAnimationPause));

    emit finished();
}

// QvisDataManager
QvisDataManager::QvisDataManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisDataManager)
{
    ui->setupUi(this);

    // Set the item type index which will restrict drops to being only to
    // parent items with the same type as the item's parent type. Thus volumes
    // can be moved to other volume groups. Similarly meshes can be moved
    // to other mesh groups. Similarly volume and mesh groups can be move to
    // other view windows. Reordering is also possible.
    ui->WorkspaceTreeWidget->setItemTypeIndex(WORKSPACE_TYPE);

    // Dummy entries.
    DatasetAdd(DatasetAttributes::Mesh,   "Some_Mesh",   "meshpath");
    DatasetAdd(DatasetAttributes::Volume, "Some_Volume", "volumepath");

    // Create an initial node for the tree
    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList({ACTIVE_DATASETS}));
    item->setData(WORKSPACE_TYPE,     Qt::UserRole, DatasetAttributes::ActiveDatasets);
    item->setData(WORKSPACE_COLOR,    Qt::UserRole, QColor{Qt::white});
    item->setData(WORKSPACE_SELECTED, Qt::UserRole, false);
    item->setData(WORKSPACE_ACTIVE,   Qt::UserRole, false);
    item->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
    item->setData(WORKSPACE_MASK,     Qt::UserRole, false);
    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    ui->WorkspaceTreeWidget->addTopLevelItem(item);

    WorkspaceViewWindowAddClicked();

    ui->WorkspaceTreeWidget->setColumnWidth(0, this->width()/2);
}

QvisDataManager::~QvisDataManager()
{
    delete ui;
}

QByteArray QvisDataManager::saveState(int version) const
{
    QByteArray xmldata;
    QXmlStreamWriter s(&xmldata);

    s.writeStartDocument();
        s.writeStartElement("QvisDataManager");
        s.writeAttribute("Version", QString::number(mCurrentVersion));
        s.writeAttribute("UserVersion", QString::number(version));

        s.writeEndElement();
    s.writeEndDocument();

    return xmldata;
}

bool QvisDataManager::restoreState(const QByteArray &state, int version)
{
    if(state.isEmpty())
        return false;

    // Prevent multiple calls as long as state is not restore. This may
    // happen, if QApplication::processEvents() is called somewhere
    if (mRestoringState)
        return false;

    mRestoringState = true;

    QXmlStreamReader s(state);

    // Read the start element - note the underscore, no spaces.
    s.readNextStartElement();

    if (s.name() != tr("QvisDataManager"))
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

    mRestoringState = false;

    return true;
}

void QvisDataManager::saveState(QSettings &Settings) const
{
    //Settings.setValue("QvisDataManager", this->saveState());

    Settings.setValue("QvisDataManager/MainSplitter", ui->MainSplitter->saveState());
}

void QvisDataManager::restoreState(const QSettings &Settings)
{
    //this->restoreState(Settings.value("QvisDataManager").toByteArray());

    ui->MainSplitter->restoreState(Settings.value("QvisDataManager/MainSplitter").toByteArray());
}

// Source buttons
void QvisDataManager::ProjectOpenClicked(QString filename)
{
    if(filename.isEmpty())
    {
        filename = QFileDialog::getOpenFileName(this,
                                                tr("Open Project"),
                                                tr(""),
                                                tr("*.vrp"));
    }
    // Make sure the name exists and is not a directory.
    else if(!std::filesystem::exists(filename.toStdString()) || std::filesystem::is_directory(filename.toStdString()))
    {
        QString message("The project, " + filename + " does not exist.");
        emit postErrorMessage(message);

        return;
    }

    if(filename.isEmpty())
        return;

    QString directory;

    if(filename.lastIndexOf("/") == -1)
        directory = ".";
    else
        directory = filename.left(filename.lastIndexOf("/"));

    emit projectDirectoryChanged(directory);

    haveProject = true;
}

void QvisDataManager::ProjectSaveClicked()
{
    // If no projcet then save as.
    if(!haveProject)
        ProjectSaveAsClicked();
}

void QvisDataManager::ProjectSaveAsClicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project as"),
                                                    tr(""),
                                                    tr("*.vrp"));

    if(filename.isEmpty())
        return;

    QString directory;

    if(filename.lastIndexOf("/") == -1)
        directory = ".";
    else
        directory = filename.left(filename.lastIndexOf("/"));

    emit projectDirectoryChanged(directory);

    haveProject = true;
}

void QvisDataManager::VolumeOpenClicked(QString filename)
{
    if(filename.isEmpty())
    {
        QFileDialog dialog(this, "Open Volume");

#ifdef __APPLE__
        const QStringList filters({"All Supported (*.tif *.tiff *.lif *.lof *.oib *.oif *.xml *.lsm *.czi *.nrrd *.vvd)",
                                   "Tiff Files (*.tif *.tiff)",
                                   "Leica Image File Format (*.lif)",
                                   "Leica Microsystems Object File Format (*.lof)",
                                   "Olympus Image Binary Files (*.oib)",
                                   "Olympus Original Imaging Format (*.oif)",
                                   "Bruker/Prairie View XML (*.xml)",
                                   "Zeiss Laser Scanning Microscope (*.lsm)",
                                   "Zeiss ZISRAW File Format (*.czi)",
                                   "Utah Nrrd files (*.nrrd)",
                                   "Janelia Brick files (*.vvd)",
                                  });
#else
        const QStringList filters({"All Supported (*.tif *.tiff *.lif *.lof *.nd2 *.oib *.oif *.xml *.lsm *.czi *.nrrd *.vvd)",
                                   "Tiff Files (*.tif *.tiff)",
                                   "Leica Image File Format (*.lif)",
                                   "Leica Microsystems Object File Format (*.lof)",
                                   "Nikon ND2 File Format (*.nd2)",
                                   "Olympus Image Binary Files (*.oib)",
                                   "Olympus Original Imaging Format (*.oif)",
                                   "Bruker/Prairie View XML (*.xml)",
                                   "Zeiss Laser Scanning Microscope (*.lsm)",
                                   "Zeiss ZISRAW File Format (*.czi)",
                                   "Utah Nrrd files (*.nrrd)",
                                   "Janelia Brick files (*.vvd)",
                                  });
#endif
        dialog.setNameFilters(filters);

        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList filenames = dialog.selectedFiles();

            if(filenames.empty() || filenames.size() > 1)
                return;

            filename = filenames[0];
        }
    }
    // Make sure the name exists and is not a directory.
    else if(!std::filesystem::exists(filename.toStdString()) || std::filesystem::is_directory(filename.toStdString()))
    {
        QString message("The volume file, " + filename + " does not exist.");
        emit postErrorMessage(message);

        return;
   }

    if(filename.isEmpty())
        return;
}

void QvisDataManager::MeshOpenClicked(QString filename)
{
    if(filename.isEmpty())
    {
        filename = QFileDialog::getOpenFileName(this,
                                                tr("Open Mesh"),
                                                tr(""),
                                                tr("*.obj"));
    }

    // Make sure the name exists and is not a directory.
    else if(!std::filesystem::exists(filename.toStdString()) || std::filesystem::is_directory(filename.toStdString()))
    {
        QString message("The mesh, " + filename + " does not exist.");
        emit postErrorMessage(message);

        return;
    }

    if(filename.isEmpty())
        return;
}

// Dataset view
void QvisDataManager::DatasetSelected(int row, int column)
{
    Q_UNUSED(column);

    // The selction is the whole row so the column is unused.
    mCurrentDataSetIndex = row;

    QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);
    mCurrentDataSet = tableItem->text();

    DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << mCurrentDataSet.toStdString() << "'  " << datasetType << std::endl;

    // Only allow one dataset to be selected.
    DatasetUpdate(mCurrentDataSetIndex, true);

    // When selecting a dataset, if present in the workspace tree select it too.
    mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentDataSet);

    // If the dataset is not in the tree check if there is a group
    // that can be selected.
    if(mCurrentTreeWidgetItem == nullptr)
    {
        // By selecting the active datasets if there is a single view window, volume
        // or mesh gruop it will be set as current. This step allow something to be
        // selected so the user does not have to do so.
        mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentViewWindow);
        WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

        if(datasetType == DatasetAttributes::Volume && mCurrentVolumeGroup.size())
            mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentVolumeGroup);
        else if(datasetType == DatasetAttributes::Mesh && mCurrentMeshGroup.size())
            mCurrentTreeWidgetItem = GetTreeWidgetItem(mCurrentMeshGroup);
    }

    // Select the current item in the tree view.
    WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);
}

void QvisDataManager::DatasetDoubleClicked(QTableWidgetItem *item)
{
    // Get the current data set as it is needed for renaming.
    mCurrentDataSetIndex = item->row();
    mCurrentDataSet = item->text();
}

void QvisDataManager::DatasetChanged(int row, int column)
{
    QTableWidgetItem *item = ui->DatasetTableWidget->item(row, DATASET_NAME);
    QString newName = item->text(); // New name

    // The only time a dataset changes is if the name is edited.
    if(column != DATASET_NAME || newName == mCurrentDataSet)
        return;

    QString tmpName = newName.simplified(); // New name with whitespace cleanup.
    tmpName.remove(" "); // Remove all whitespace;

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentDataSet.toStdString() << "  " << tmpName.toStdString() << std::endl;

    // Make sure the new dataset name is different than other dataset names.
    for(int i=0; i<ui->DatasetTableWidget->rowCount(); ++i)
    {
        QString tmp = ui->DatasetTableWidget->item(i, DATASET_NAME)->text().simplified();
        tmp.remove(" "); // Remove all whitespace;

        if(i != row && tmpName == tmp)
        {
            const QSignalBlocker blocker(ui->DatasetTableWidget);
            item->setText(mCurrentDataSet);

            QString message("Cannot rename the dataset because the name is already in use.");
            emit postErrorMessage(message);

            return;
        }
    }

    DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

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
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(treeItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());
        if(datasetType != dType)
            continue;

        // Only rename datasets with the same base name.
        QString name = treeItem->text(WORKSPACE_NAME);
        QString suffix("");

        // Rename datasets with the same name.
        if(name == mCurrentDataSet)
        {
            suffix = tr("");
        }
        // Change datasets with the same base name and get the suffix "_N" where N is a number.
        else if(checkBaseName &&  name.startsWith(mCurrentDataSet))
        {
            suffix = name.last(name.size()-mCurrentDataSet.size());

            // Make sure there is an underscore followed by an integer.
            if(!suffix.startsWith("_") || suffix.last(suffix.size()-1).toInt() == 0)
                continue;

            if(yesToAll == false)
            {
                QMessageBox dialog;
                dialog.setText("Datasets that have the same base name, '" + mCurrentDataSet + "' have been found.");
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
        treeItem->setText(WORKSPACE_NAME, newName + suffix);

        // Update the current selection with the new name.
        if(datasetType == DatasetAttributes::Mesh && mCurrentVolume == name)
            mCurrentVolume = newName + suffix;
        else if(datasetType == DatasetAttributes::Volume && mCurrentMesh == name)
            mCurrentMesh = newName + suffix;

        // Update the last selection list with the new name.
        QString viewWindow  = treeItem->parent()->parent()->text(WORKSPACE_NAME);
        QString parentGroup = treeItem->parent()->text(WORKSPACE_NAME);

        mLastSelections[viewWindow].datasets[parentGroup] = newName + suffix;

        emit updateViewWindow(viewWindow);
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentDataSet.toStdString() << "  " << newName.toStdString() << std::endl;

    mCurrentDataSetIndex = item->row();
    mCurrentDataSet = item->text();
}

void QvisDataManager::DatasetContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = ui->DatasetTableWidget->itemAt(pos);

    if(item)
    {
        mCurrentDataSetIndex = item->row();
        mCurrentDataSet = item->text();
        DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                    DatasetAttributes::Volume : DatasetAttributes::Mesh;

        QMenu *contextMenu = new QMenu(this);

        // Add the currently selected view window and if present volume and mesh groups.
        QMenu *viewWindowMenu = GetTreeWidgetMenu(contextMenu, datasetType);
        contextMenu->addMenu(viewWindowMenu);

        // Menu items common to all datasets.
        contextMenu->addAction(QString("Rename"),               this, SLOT(DatasetRenameClicked()));
        contextMenu->addAction(QString("Save As ..."),          this, SLOT(DatasetSaveAsClicked()));
        contextMenu->addAction(QString("Delete"),               this, SLOT(DatasetDeleteClicked()));

        // Menu items common to volumes only.
        if(datasetType == DatasetAttributes::Volume)
        {
            contextMenu->addAction(QString("Apply Properties ..."), this, SLOT(DatasetApplyPropertiesClicked()));
            contextMenu->addAction(QString("Save Mask ..."),        this, SLOT(DatasetSaveMaskClicked()));
        }

        contextMenu->popup(ui->DatasetTableWidget->viewport()->mapToGlobal(pos));
    }
}

// Dataset buttons
void QvisDataManager::DatasetAddClicked(const QString name)
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.size() == 0)
    {
        QString message("Cannot add a dataset because a dataset is not selected.");
        emit postErrorMessage(message);

        return;
    }

    QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);
    DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << name.toStdString() << "'  "  << "  '" << tableItem->text().toStdString() << "'  " << datasetType << std::endl;

    QTreeWidgetItem * parent = nullptr;

    // If a name then the signal is from a menu action so use the name to get the parent group.
    if(name.size())
    {
        parent = GetTreeWidgetItem(name);
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(parent->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

        if(dType == DatasetAttributes::ViewWindow)
        {
            if(mCurrentViewWindow != name)
            {
                mCurrentViewWindow = name;
                emit activeViewWindowChanged(mCurrentViewWindow);
            }

            if(datasetType == DatasetAttributes::Volume)
                parent = WorkspaceVolumeGroupAddClicked();
            else if(datasetType == DatasetAttributes::Mesh)
                parent = WorkspaceMeshGroupAddClicked();
        }
        else if(dType == DatasetAttributes::VolumeGroup && datasetType == DatasetAttributes::Volume)
        {
            mCurrentVolumeGroup = name;
        }
        else if(dType == DatasetAttributes::MeshGroup && datasetType == DatasetAttributes::Mesh)
        {
            mCurrentMeshGroup = name;
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
        if(parent == nullptr && mCurrentViewWindow.size())
        {
            QTreeWidgetItem *rvItem = GetTreeWidgetItem(mCurrentViewWindow);

            // No volume or mesh group may be current because there can be mutliple
            // volume or mesh groups and it is abiguous as to which one is current.
            // As such, only create a group if the are no groups.
            int nVolumes = 0;
            int nMeshes = 0;

            for(int i=0; i<rvItem->childCount(); ++i)
            {
                QTreeWidgetItem *child = rvItem->child(i);
                DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

                if(dType == DatasetAttributes::VolumeGroup)
                    ++nVolumes;
                else if(dType == DatasetAttributes::MeshGroup)
                    ++nMeshes;
            }

            // If there is not a group then create one.
            if(datasetType == DatasetAttributes::Volume && nVolumes == 0)
                parent = WorkspaceVolumeGroupAddClicked();
            else if(datasetType == DatasetAttributes::Mesh && nMeshes == 0)
                parent = WorkspaceMeshGroupAddClicked();
        }
    }

    // Have a parent group so can now add the dataset.
    if(parent)
    {
        if(datasetType == DatasetAttributes::Volume ||
           datasetType == DatasetAttributes::Mesh)
        {
            QString tmp = WorkspaceExists(tableItem->text(), datasetType);

            QColor color(Qt::red); // FIX ME - get a random color.

            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

            QTreeWidgetItem * child = new QTreeWidgetItem(QStringList({tmp, "", ""}));
            child->setData(WORKSPACE_TYPE,     Qt::UserRole, datasetType);
            child->setData(WORKSPACE_COLOR,    Qt::UserRole, color);
            child->setData(WORKSPACE_SELECTED, Qt::UserRole, false);
            child->setData(WORKSPACE_ACTIVE,   Qt::UserRole, false);
            child->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
            child->setData(WORKSPACE_MASK,     Qt::UserRole, false);
            child->setIcon(WORKSPACE_NAME,  GetLightIcon(color));
            child->setIcon(WORKSPACE_COLOR, GetColorIcon(color));

            parent->addChild(child);
            ExpandParent(child);

            mCurrentTreeWidgetItem = child;
            WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

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

void QvisDataManager::DatasetRenameClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.size() == 0)
    {
        QString message("Cannot rename a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }

    QTableWidgetItem *item = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);

    if(item)
        ui->DatasetTableWidget->editItem(item);
}

void QvisDataManager::DatasetSaveAsClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.size() == 0)
    {
        QString message("Cannot save a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDataManager::DatasetDeleteClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.isEmpty())
    {
        QString message("Cannot delete a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }

    DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    // Delete the dataset from the table.
    ui->DatasetTableWidget->removeRow(mCurrentDataSetIndex);

    // Traverse the workspace tree and delete the dataset name(s).
    // Note if a dataset is present multiple times the root name can be
    // deleted for ALL instances.
    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    // Hold the current dataset as it can change when deleted from the tree.
    QString currentDataSet = mCurrentDataSet;

    bool checkBaseName = true;
    bool yesToAll = false;

    // If present, delete the dataset from the tree.
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QTreeWidgetItem *treeItem = *it;
        it++;

        // Only delete datasets with the same type.
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(treeItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());
        if(datasetType != dType)
            continue;

        // Only delete datasets with the same base name.
        const QString name = treeItem->text(WORKSPACE_NAME);

        // Delete the dataset with the same name.
        if(name == currentDataSet)
        {
        }
        // Only delete datasets with the same base name and get the suffix "_N" where N is a number.
        else if(checkBaseName && name.startsWith(currentDataSet))
        {
            QString suffix = name.last(name.size()-currentDataSet.size());

            // Make sure there is an underscore followed by an integer.
            if(!suffix.startsWith("_") || suffix.last(suffix.size()-1).toInt() == 0)
                continue;

            if(yesToAll == false)
            {
                QMessageBox dialog;
                dialog.setText("Datasets that have the same base name, '" + currentDataSet + "' have been found.");
                dialog.setInformativeText("Do you want to delete all of these datasets too?\n\n"
                                          "Ok to just delete '" + name + "'.");
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

        WorkspaceSelectionDeleteClicked(name);
    }

    mCurrentDataSetIndex = -1;
    mCurrentDataSet = tr("");
}

void QvisDataManager::DatasetDeleteAllClicked()
{
    QMessageBox dialog;
    dialog.setText("Confirm - Delete all datasets?");
    dialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    if(dialog.exec() != QMessageBox::Ok)
        return;

    // Delete the table row by row so to also delete the tree.
    while(ui->DatasetTableWidget->rowCount())
    {
        mCurrentDataSetIndex = 0;
        mCurrentDataSet = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME)->text();

        DatasetDeleteClicked();
    }

    mCurrentDataSetIndex = -1;
    mCurrentDataSet = tr("");
}

void QvisDataManager::DatasetApplyPropertiesClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.size() == 0)
    {
        QString message("Cannot apply properties to a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDataManager::DatasetSaveMaskClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSet.size() == 0)
    {
        QString message("Cannot save a dataset mask because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDataManager::DatasetAdd(DatasetAttributes::DatasetType type, const QString &name, const QString &path)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << name.toStdString() << "'  " << type << std::endl;

    const QSignalBlocker blocker(ui->DatasetTableWidget);

    // First insert a new row.
    int row = ui->DatasetTableWidget->rowCount();
    ui->DatasetTableWidget->insertRow(row);

    QTableWidgetItem *item = nullptr;

    // Now add the type, name, and path.
    if (type == DatasetAttributes::Volume)
        item = new QTableWidgetItem("Volume");
    else if (type == DatasetAttributes::Mesh)
        item = new QTableWidgetItem("Mesh");
    else if (type == DatasetAttributes::Annotations)
        item = new QTableWidgetItem("Annotations");

    if(item)
    {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 0, item);

        item = new QTableWidgetItem(name);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 1, item);

        item = new QTableWidgetItem(path);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 2, item);
    }
}

// Update the dataset that is to be selected. Only one can be selected.
void QvisDataManager::DatasetUpdate(int index, bool value)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << index << "  " << value << std::endl;

    const QSignalBlocker blocker(ui->DatasetTableWidget);

    for(int row=0; row<ui->DatasetTableWidget->rowCount(); ++row)
    {
        for(int col=0; col<ui->DatasetTableWidget->columnCount(); ++col)
        {
            QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(row, col);

            if(row == index)
            {
                if(tableItem->isSelected() != value)
                    tableItem->setSelected(value);
            }
            else
            {
                if(tableItem->isSelected() != false)
                    tableItem->setSelected(false);
            }
        }
    }
}

QTableWidgetItem * QvisDataManager::GetTableWidgetItem(const QString &name, bool exact) const
{
    if(name.isEmpty())
        return nullptr;

    // Helper to get the table item based on the name. The name maybe exact (default) or
    // based on the name as a base.
    for(int row=0; row<ui->DatasetTableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(row, DATASET_NAME);

        QString tmp = tableItem->text();

        if((exact && tmp == name) || (!exact && tmp.startsWith(name)))
            return tableItem;
    }

    return nullptr;
}


// Animation
void QvisDataManager::AnimationIndexSliderReleased()
{
    if(mAnimationSliderMode == DatasetAttributes::SingleStep)
    {
        mAnimationStop = false;
        mAnimationContinuous = false;
        mAnimationStep = 0;

        Animation();
    }
}

void QvisDataManager::AnimationSliderModeChanged(int value)
{
    mAnimationSliderMode = DatasetAttributes::AnimationSliderMode(value);
}

void QvisDataManager::AnimationPauseValueChanged(int value)
{
    mAnimationPause = value;
}

void QvisDataManager::AnimationIndexSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->AnimationIndexSpinBox);
    ui->AnimationIndexSpinBox->setValue(value);
    ui->AnimationIndexSpinBox->setFocus();

    if(mAnimationSliderMode == DatasetAttributes::Continuous)
    {
        mAnimationStop = false;
        mAnimationContinuous = false;
        mAnimationStep = 0;

        Animation();
    }
}

void QvisDataManager::AnimationIndexValueChanged(int value)
{
    const QSignalBlocker blocker(ui->AnimationIndexSlider);
    ui->AnimationIndexSlider->setValue(value);

    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = 0;

    Animation();
}
void QvisDataManager::AnimationReverseSingleStepClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = -1;

    Animation();
}

void QvisDataManager::AnimationReverseClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = true;
    mAnimationStep = -1;

    Animation();
}

void QvisDataManager::AnimationStopClicked()
{
    mAnimationStop = true;
    mAnimationContinuous = false;
    mAnimationStep = 0;
}

void QvisDataManager::AnimationPlayClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = true;
    mAnimationStep = 1;

    Animation();
}

void QvisDataManager::AnimationForwardSingleStepClicked()
{

    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = 1;

    Animation();
}

void QvisDataManager::Animation()
{
    // First update the counter.
    if(mAnimationStep)
    {
        int value = ui->AnimationIndexSlider->value() + mAnimationStep;

        if(ui->AnimationIndexSpinBox->minimum() <= value &&
           value <= ui->AnimationIndexSpinBox->maximum())
        {
            const QSignalBlocker blocker0(ui->AnimationIndexSpinBox);
            ui->AnimationIndexSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->AnimationIndexSlider);
            ui->AnimationIndexSlider->setValue(value);
        }
        else
        {
            ui->VCRWidget->StopClicked();
        }
    }

    // If the animation state is not stopped create a thread and worker.
    // do the animation A thread is necessary so to allow the continued
    // processing of UI events. Namely allowing the user to stop the animation.
    if(mAnimationStop == false)
    {
        QvisAnimationWorker* worker = new QvisAnimationWorker();

        worker->mAnimationPause = mAnimationPause;

        QThread* thread = new QThread();

        // Assign the worker to the thread.
        worker->moveToThread(thread);

        // If running continuously animate the next frame otherwise stop.
        if(mAnimationContinuous)
            connect(worker, SIGNAL(finished()), this, SLOT(Animation()));
        else
            mAnimationStop = true;

        // When the thread is started the worker process begins.
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        // When the worker is finished the thread quits.
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        // Take care of cleaning up when finished too
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        // Start the thread.
        thread->start();
    }
}

// Workspace view
void QvisDataManager::WorkspaceSelected(QTreeWidgetItem *item, int column)
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

    QString text = mCurrentTreeWidgetItem->text(WORKSPACE_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

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
        QTableWidgetItem * tableItem = GetTableWidgetItem(text);

        if(tableItem != nullptr)
        {
            mCurrentDataSet = tableItem->text();
            DatasetUpdate(tableItem->row(), true);
        }
        // This condition happens when the dataset selected is a "copy" of the original.
        else
        {
            mCurrentDataSet = "";
            DatasetUpdate(-1, false);
        }
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '"
              << text.toStdString() << "'  '"
              << mCurrentViewWindow.toStdString() << "'  '"
              << mCurrentVolumeGroup.toStdString() << "'  '"
              << mCurrentVolume.toStdString() << "'  '"
              << mCurrentMeshGroup.toStdString() << "'  '"
              << mCurrentMesh.toStdString() << "'  "
              << std::endl;

    WorkspaceUpdate(WORKSPACE_SELECTED, true);
}

void QvisDataManager::ViewWindowSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::ViewWindow)
    {
        if(mCurrentViewWindow != item->text(WORKSPACE_NAME))
        {
            mCurrentViewWindow = item->text(WORKSPACE_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        bool multpleVolumes = false;
        bool multpleMeshes = false;

        // Set the current volume and/or mesh group. But only if there is one of each.
        // Otherwise it is ambiguous.
        for(int i=0; i<item->childCount(); ++i)
        {
            QTreeWidgetItem *child = item->child(i);
            QString text = child->text(WORKSPACE_NAME);
            DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

            if(dType == DatasetAttributes::VolumeGroup && !multpleVolumes)
            {
                // Only set the current volume group if there is one volume group
                if(mCurrentVolumeGroup == tr(""))
                {
                    mCurrentVolumeGroup = text;

                    // Only set the current volume if there is one volume
                    if(child->childCount() == 1)
                        mCurrentVolume = child->child(0)->text(WORKSPACE_NAME);
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
                        mCurrentMesh = child->child(0)->text(WORKSPACE_NAME);
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

void QvisDataManager::VolumeGroupSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::VolumeGroup)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(WORKSPACE_NAME))
        {
            mCurrentViewWindow = item->parent()->text(WORKSPACE_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentMeshGroup  = mLastSelections[mCurrentViewWindow].meshGroup;
        mCurrentMesh       = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];

        // Now set the volume group.
        mCurrentVolumeGroup = item->text(WORKSPACE_NAME);
        mLastSelections[mCurrentViewWindow].volumeGroup = mCurrentVolumeGroup;

        // Check to see if the current volume is in the current volume group.
        mCurrentVolume = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];
        QTreeWidgetItem *volumeItem = GetTreeWidgetItem(mCurrentVolume);

        if(volumeItem == nullptr || volumeItem->parent()->text(WORKSPACE_NAME) != mCurrentVolumeGroup)
        {
            // Only set the current volume if there is one volume
            if(item->childCount() == 1)
                mCurrentVolume = item->child(0)->text(WORKSPACE_NAME);
            else
                mCurrentVolume = tr("");

            mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup] = mCurrentVolume;
        }
    }
}

void QvisDataManager::MeshGroupSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::MeshGroup)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(WORKSPACE_NAME))
        {
            mCurrentViewWindow = item->parent()->text(WORKSPACE_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentVolumeGroup = mLastSelections[mCurrentViewWindow].volumeGroup;
        mCurrentVolume      = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];

        // Now set the mesh group.
        mCurrentMeshGroup = item->text(WORKSPACE_NAME);
        mLastSelections[mCurrentViewWindow].meshGroup = mCurrentMeshGroup;

        // Check to see if the current mesh is in the current mesh group.
        mCurrentMesh = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];
        QTreeWidgetItem *meshItem = GetTreeWidgetItem(mCurrentMesh);

        if(meshItem == nullptr || meshItem->parent()->text(WORKSPACE_NAME) != mCurrentMeshGroup)
        {
            // Only set the current mesh if there is one mesh
            if(item->childCount() == 1)
                mCurrentMesh = item->child(0)->text(WORKSPACE_NAME);
            else
                mCurrentMesh = tr("");

            mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup] = mCurrentMesh;
        }
    }
}

void QvisDataManager::VolumeSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::Volume)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(WORKSPACE_NAME))
        {
            mCurrentViewWindow = item->parent()->parent()->text(WORKSPACE_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentMeshGroup  = mLastSelections[mCurrentViewWindow].meshGroup;
        mCurrentMesh       = mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup];

        // Now set the volume group and volume.
        mCurrentVolumeGroup = item->parent()->text(WORKSPACE_NAME);
        mCurrentVolume      = item->text(WORKSPACE_NAME);

        emit activeViewWindowVolumeChanged(mCurrentViewWindow, mCurrentVolume);

        mLastSelections[mCurrentViewWindow].volumeGroup = mCurrentVolumeGroup;
        mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup] = mCurrentVolume;
    }
}

void QvisDataManager::MeshSelected(QTreeWidgetItem *item)
{
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType == DatasetAttributes::Mesh)
    {
        // Get the upstream view window and sibling mesh group and mesh names.
        if(mCurrentViewWindow != item->parent()->text(WORKSPACE_NAME))
        {
            mCurrentViewWindow = item->parent()->parent()->text(WORKSPACE_NAME);
            emit activeViewWindowChanged(mCurrentViewWindow);
        }

        mCurrentVolumeGroup = mLastSelections[mCurrentViewWindow].volumeGroup;
        mCurrentVolume      = mLastSelections[mCurrentViewWindow].datasets[mCurrentVolumeGroup];

        // Now set the mesh group and mesh.
        mCurrentMeshGroup  = item->parent()->text(WORKSPACE_NAME);
        mCurrentMesh       = item->text(WORKSPACE_NAME);

        emit activeViewWindowMeshChanged(mCurrentViewWindow, mCurrentMesh);

        mLastSelections[mCurrentViewWindow].meshGroup = mCurrentMeshGroup;
        mLastSelections[mCurrentViewWindow].datasets[mCurrentMeshGroup] = mCurrentMesh;
    }
}

void QvisDataManager::WorkspaceIconClicked(QTreeWidgetItem *item, int column)
{
    if(item == mCurrentTreeWidgetItem && column == WORKSPACE_NAME)
    {
        WorkspaceHideShowClicked();
    }
    else if(item == mCurrentTreeWidgetItem && column == WORKSPACE_COLOR)
    {
        WorkspaceRandomizeColors();
    }
}

void QvisDataManager::WorkspaceDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    // Emit a signal to that will ALWAYS bring up the window.
    if(item == mCurrentTreeWidgetItem) // && column == WORKSPACE_NAME)
    {
        if(dType == DatasetAttributes::ViewWindow)
            emit mViewWindowActionList[0]->triggered(true);
        else if(dType == DatasetAttributes::Volume)
            emit mVolumeActionList[0]->triggered(true);
        else if(dType == DatasetAttributes::Mesh)
            emit mMeshActionList[0]->triggered(true);
    }
}

void QvisDataManager::WorkspaceItemMoved(QTreeWidgetItem *item,   int iColumn,
                                         QTreeWidgetItem *parent, int pColumn)
{
    Q_UNUSED(iColumn);
    Q_UNUSED(pColumn);

    if(DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt()) != DatasetAttributes::ViewWindow)
    {
        QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(WORKSPACE_NAME));
        emit updateViewWindow(viewItem->text(WORKSPACE_NAME));

        viewItem = GetTreeWidgetItemViewWindow(parent->text(WORKSPACE_NAME));
        emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
    }

    int index = item->parent()->indexOfChild(item);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '"
              << item->text(0).toStdString()
              << "'  new parent '"
              << item->parent()->text(0).toStdString() << "'  " << index
              << "  old parent '"
              << parent->text(0).toStdString() << "'  " << std::endl;
}

void QvisDataManager::WorkspaceChanged(QTreeWidgetItem *item, int column)
{
    if(WORKSPACE_NAME != column)
        return;

    // Only volume and mesh groups can be renamed.
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());
    if((dType & (DatasetAttributes::VolumeGroup | DatasetAttributes::MeshGroup)) == 0)
        return;

    QString newName = item->text(WORKSPACE_NAME);

    QString tmpName = newName.simplified(); // New name with whitespace cleanup.
    tmpName.remove(" "); // Remove all whitespace;

    // Check to make sure the name is unique.
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QString tmp = (*it)->text(WORKSPACE_NAME).simplified();
        tmp.remove(" "); // Remove all whitespace;

        if((*it) != item && tmpName == tmp)
        {
            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

            if(dType == DatasetAttributes::VolumeGroup)
                item->setText(WORKSPACE_NAME, mCurrentVolumeGroup);
            else if(dType == DatasetAttributes::MeshGroup)
                item->setText(WORKSPACE_NAME, mCurrentMeshGroup);

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

    WorkspaceSelected(item, WORKSPACE_NAME);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << item->text(WORKSPACE_NAME).toStdString() << "'  " << dType << std::endl;
}

void QvisDataManager::ExpandParent(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();

    if(parent)
    {
        parent->setExpanded(true);
        ExpandParent(parent);
    }
}

QTreeWidgetItem * QvisDataManager::GetTreeWidgetItem(const QString &name, bool exact) const
{
    if(name.isEmpty())
        return nullptr;

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    while(*it)
    {
        QString tmp = (*it)->text(WORKSPACE_NAME);

        if((exact && tmp == name) || (!exact && tmp.startsWith(name)))
            return (*it);

        ++it;
    }

    return nullptr;
}

QTreeWidgetItem * QvisDataManager::GetTreeWidgetItemViewWindow(const QString &name) const
{
    if(name.isEmpty())
        return nullptr;

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);
    QTreeWidgetItem *view = nullptr;

    while(*it)
    {
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType((*it)->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

        if(dType == DatasetAttributes::ViewWindow)
            view = *it;

        QString tmp = (*it)->text(WORKSPACE_NAME);

        if(tmp == name)
            return (view);

        ++it;
    }

    return nullptr;
}

QMenu * QvisDataManager::GetTreeWidgetMenu(QWidget *parent, const DatasetAttributes::DatasetType datasetType)
{
    QMenu *menu = new QMenu(parent);
    menu->setTitle("Add to");

    // Use a QSignalMapper which will map the view window name and volume/mesh group name to the DatasetAddClicked slot.
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(DatasetAddClicked(QString)));

    for(int i=0; i<ui->WorkspaceTreeWidget->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem * item = ui->WorkspaceTreeWidget->topLevelItem(i);

        GetTreeWidgetMenu(datasetType, item, menu, signalMapper);
    }

    return menu;
}

void QvisDataManager::GetTreeWidgetMenu(const DatasetAttributes::DatasetType datasetType, const QTreeWidgetItem *item, QMenu *menu, QSignalMapper* signalMapper) const
{
    QString text = item->text(WORKSPACE_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

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
            DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(child->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

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

void QvisDataManager::WorkspaceContextMenuRequested(const QPoint &pos)
{
    mCurrentTreeWidgetItem = ui->WorkspaceTreeWidget->itemAt(pos);

    if(mCurrentTreeWidgetItem == nullptr)
        return;

    WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

    QString text = mCurrentTreeWidgetItem->text(WORKSPACE_NAME);
    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());
    bool visible = mCurrentTreeWidgetItem->data(WORKSPACE_VISIBILE, Qt::UserRole).toBool();
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

        contextMenu->addAction("Add Volume Group", this, SLOT(WorkspaceVolumeGroupAddClicked()));
        contextMenu->addAction("Add Mesh Group",   this, SLOT(WorkspaceMeshGroupAddClicked()));

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
    else if(dType == DatasetAttributes::VolumeGroup ||
            dType == DatasetAttributes::MeshGroup)
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
    else if(dType == DatasetAttributes::Volume ||
            dType == DatasetAttributes::Mesh)
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

void QvisDataManager::WorkspaceToggleTreeWidgetItem()
{
    mCurrentTreeWidgetItem->setExpanded(!mCurrentTreeWidgetItem->isExpanded());
}

void QvisDataManager::WorkspaceShowAll(QTreeWidgetItem *item)
{
    // The initial call is from the menu thus item is null so use the current item.
    // Subsuquent recursive calls the item should be valid.
    if(item == nullptr)
        item = mCurrentTreeWidgetItem;

    if(item == nullptr)
        return;

    // Make the current item visible.
    QColor color = item->data(WORKSPACE_COLOR, Qt::UserRole).value<QColor>();
    item->setIcon(WORKSPACE_NAME, GetLightIcon(color));
    mCurrentTreeWidgetItem->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);

    // Make all of the children visible.
    for(int i=0; i<item->childCount(); i++)
    {
        WorkspaceShowAll(item->child(i));
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(WORKSPACE_NAME));
    emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
}

void QvisDataManager::WorkspaceIsolate()
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
        bool visible = child->data(WORKSPACE_VISIBILE, Qt::UserRole).toBool();

        if(child == mCurrentTreeWidgetItem)
        {
            if(!visible)
            {
                QColor color = child->data(WORKSPACE_COLOR, Qt::UserRole).value<QColor>();

                child->setIcon(WORKSPACE_NAME, GetLightIcon(color));
                child->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
            }
        }
        else if(visible)
        {
            child->setIcon(WORKSPACE_NAME, GetLightIcon(QColor(Qt::black)));
            child->setData(WORKSPACE_VISIBILE, Qt::UserRole, false);
        }
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(parent->text(WORKSPACE_NAME));
    emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
}

void QvisDataManager::WorkspaceRandomizeColors(QTreeWidgetItem *item)
{
    // The initial call is from the menu thus item is null so use the current item.
    // Subsuquent recursive calls the item should be valid.
    if(item == nullptr)
        item = mCurrentTreeWidgetItem;

    if(item == nullptr)
        return;

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(item->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(item->childCount())
    {
        for(int i=0; i<item->childCount(); i++)
        {
            WorkspaceRandomizeColors(item->child(i));
        }
    }
    else if(dType == (DatasetAttributes::Volume | DatasetAttributes::Mesh))
    {
        QColor color(Qt::green); // FIX ME - get a random color.
        item->setData(WORKSPACE_COLOR, Qt::UserRole, color);

        bool visible = item->data(WORKSPACE_VISIBILE, Qt::UserRole).toBool();

        if(visible)
            item->setIcon(WORKSPACE_NAME, GetLightIcon(color));
        else if(visible)
            item->setIcon(WORKSPACE_NAME, GetLightIcon(QColor(Qt::black)));

        item->setIcon(WORKSPACE_COLOR, GetColorIcon(color));
    }

    QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(item->text(WORKSPACE_NAME));
    emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
}

void QvisDataManager::WorkspaceRename()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    ui->WorkspaceTreeWidget->editItem(mCurrentTreeWidgetItem);
}

void QvisDataManager::ViewWindowToggleView()
{
    emit actionVewWindowToggleView();
}

void QvisDataManager::VolumeCopyMask()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    QString text = mCurrentTreeWidgetItem->text(WORKSPACE_NAME);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << ACTIVE_DATASETS.toStdString() << std::endl;

    WorkspaceUpdate(WORKSPACE_MASK, true);
}

void QvisDataManager::VolumePasteMask()
{

}

void QvisDataManager::VolumeMergeMask()
{

}

void QvisDataManager::VolumeExcludeMask()
{

}

void QvisDataManager::VolumeIntersectMask()
{

}

// Workspace buttons
void QvisDataManager::WorkspaceHideShowClicked()
{
    if(mCurrentTreeWidgetItem == nullptr)
        return;

    DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dType != DatasetAttributes::ActiveDatasets)
    {
        bool visible = mCurrentTreeWidgetItem->data(WORKSPACE_VISIBILE, Qt::UserRole).toBool();

        if(visible)
        {
            mCurrentTreeWidgetItem->setIcon(WORKSPACE_NAME, GetLightIcon(QColor(Qt::black)));
        }
        else
        {
            QColor color = mCurrentTreeWidgetItem->data(WORKSPACE_COLOR, Qt::UserRole).value<QColor>();

            mCurrentTreeWidgetItem->setIcon(WORKSPACE_NAME, GetLightIcon(color));
        }

        mCurrentTreeWidgetItem->setData(WORKSPACE_VISIBILE, Qt::UserRole, !visible);

        QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(mCurrentTreeWidgetItem->text(WORKSPACE_NAME));
        emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
    }
}

void QvisDataManager::WorkspaceSelectionDeleteClicked(QString name)
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

    DatasetAttributes::DatasetType dataType = DatasetAttributes::DatasetType(mCurrentTreeWidgetItem->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

    if(dataType == DatasetAttributes::ActiveDatasets)
    {
        QString message("Cannot delete the Active Datasets node.");
        emit postErrorMessage(message);
        return;
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << "  "
              << mCurrentTreeWidgetItem->text(WORKSPACE_NAME).toStdString() << "  " << std::endl;

    QTreeWidgetItem *parent = mCurrentTreeWidgetItem->parent();

    if(parent)
    {
        DatasetAttributes::DatasetType dType = DatasetAttributes::DatasetType(parent->data(WORKSPACE_TYPE, Qt::UserRole).toInt());

        std::cerr << __FUNCTION__ << "  " << __LINE__ << " parent " << parent->text(0).toStdString() << "  " << parent->childCount() << "  "
                  << dType << "  " << std::endl;

        // If the name is empty then there must be at least one view window. Otherwise
        // the main window is forcing a view window to be deleted.
        int minChildren = name.isEmpty() ? 1 : 0;
        if((dType == DatasetAttributes::ActiveDatasets && parent->childCount() > minChildren) ||
           (dType != DatasetAttributes::ActiveDatasets))
        {
            // Before deleting the item get its view, sibling, and number of child so the view can be updated if needed.
            QTreeWidgetItem *viewItem = GetTreeWidgetItemViewWindow(mCurrentTreeWidgetItem->text(WORKSPACE_NAME));
            int nChildren = mCurrentTreeWidgetItem->childCount();

            int sibling = std::min(parent->indexOfChild(mCurrentTreeWidgetItem), parent->childCount()-1);

            parent->removeChild(mCurrentTreeWidgetItem);

            if(dataType == DatasetAttributes::ViewWindow)
            {
                // If the name is empty then the signal was from the data manager so signal the main window.
                // If the name is not empty then the signal was from the main window.
                if(name.isEmpty())
                    emit actionViewWindowDelete(name);
            }
            else if((dataType == DatasetAttributes::VolumeGroup && nChildren) ||
                    (dataType == DatasetAttributes::MeshGroup && nChildren) ||
                    (dataType == DatasetAttributes::Volume) ||
                    (dataType == DatasetAttributes::Mesh))
            {
                emit updateViewWindow(viewItem->text(WORKSPACE_NAME));
            }

            // After deleting the item, the newly selected item will be a sibbling or the parent so
            // set the mCurrentTreeWidgetItem
            if(parent->childCount() == 0)
                mCurrentTreeWidgetItem = parent;
            else
                mCurrentTreeWidgetItem = parent->child(sibling);

            WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);
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

void QvisDataManager::WorkspaceViewWindowSelected(QString name)
{
    if(name != mCurrentViewWindow)
    {
        mCurrentTreeWidgetItem = GetTreeWidgetItem(name);

        // Select the current item in the tree view.
        WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << name.toStdString() << std::endl;

        // Select the current item in the dataset list.

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
    }
}

void QvisDataManager::WorkspaceViewWindowRename(QString currentName, QString newName)
{
    QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    QTreeWidgetItem *widget = GetTreeWidgetItem(currentName);

    widget->setText(WORKSPACE_NAME, newName);
}

void QvisDataManager::WorkspaceViewWindowAddClicked(QString name)
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
        QTreeWidgetItem *item = GetTreeWidgetItem(ACTIVE_DATASETS);

        if(item)
        {
            const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

            QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({name}));
            child->setData(WORKSPACE_TYPE,     Qt::UserRole, DatasetAttributes::ViewWindow);
            child->setData(WORKSPACE_COLOR,    Qt::UserRole, QColor(Qt::white));
            child->setData(WORKSPACE_SELECTED, Qt::UserRole, false);
            child->setData(WORKSPACE_ACTIVE,   Qt::UserRole, false);
            child->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
            child->setData(WORKSPACE_MASK,     Qt::UserRole, false);
            child->setIcon(WORKSPACE_NAME,  GetLightIcon(QColor(Qt::white)));
            child->setIcon(WORKSPACE_COLOR, GetColorIcon(QColor(Qt::white)));

            item->addChild(child);
            ExpandParent(child);

            mCurrentTreeWidgetItem = child;
            WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);
        }
    }
}

QTreeWidgetItem * QvisDataManager::WorkspaceVolumeGroupAddClicked()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentViewWindow.toStdString() << std::endl;

    QTreeWidgetItem *item = GetTreeWidgetItem(mCurrentViewWindow);

    if(item)
    {
        const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({VOLUME_GROUP + QString("%1").arg(mNumVolumeGroups++), "", ""}));
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        child->setData(WORKSPACE_TYPE,     Qt::UserRole, DatasetAttributes::VolumeGroup);
        child->setData(WORKSPACE_COLOR,    Qt::UserRole, QColor(Qt::white));
        child->setData(WORKSPACE_SELECTED, Qt::UserRole, false);
        child->setData(WORKSPACE_ACTIVE,   Qt::UserRole, false);
        child->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
        child->setData(WORKSPACE_MASK,     Qt::UserRole, false);
        child->setIcon(WORKSPACE_NAME,  GetLightIcon(QColor(Qt::white)));
        child->setIcon(WORKSPACE_COLOR, GetColorIcon(QColor(Qt::white)));

        item->addChild(child);
        ExpandParent(child);

        mCurrentTreeWidgetItem = child;
        WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

        return child;
    }
    else
    {
        QString message("Cannot add a mesh group, no view window is selected.");
        emit postErrorMessage(message);

        return nullptr;
    }
}

QTreeWidgetItem * QvisDataManager::WorkspaceMeshGroupAddClicked()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentViewWindow.toStdString() << std::endl;

    QTreeWidgetItem *item = GetTreeWidgetItem(mCurrentViewWindow);

    if(item)
    {
        const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList({MESH_GROUP + QString("%1").arg(mNumMeshGroups++), "", ""}));
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        child->setData(WORKSPACE_TYPE,     Qt::UserRole, DatasetAttributes::MeshGroup);
        child->setData(WORKSPACE_COLOR,    Qt::UserRole, QColor(Qt::white));
        child->setData(WORKSPACE_SELECTED, Qt::UserRole, false);
        child->setData(WORKSPACE_ACTIVE,   Qt::UserRole, false);
        child->setData(WORKSPACE_VISIBILE, Qt::UserRole, true);
        child->setData(WORKSPACE_MASK,     Qt::UserRole, false);
        child->setIcon(WORKSPACE_NAME,  GetLightIcon(QColor(Qt::white)));
        child->setIcon(WORKSPACE_COLOR, GetColorIcon(QColor(Qt::white)));

        item->addChild(child);
        ExpandParent(child);

        mCurrentTreeWidgetItem = child;
        WorkspaceSelected(mCurrentTreeWidgetItem, WORKSPACE_NAME);

        return child;
    }
    else
    {
        QString message("Cannot add a mesh group, no view window is selected.");
        emit postErrorMessage(message);

       return nullptr;
    }
}

void QvisDataManager::WorkspaceUpdate(int index, bool value)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << index << std::endl;

    const QSignalBlocker blocker(ui->WorkspaceTreeWidget);

    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    // Set the values so at most ONE is selected or is the mask.
    while(*it)
    {
        bool val = (*it)->data(index, Qt::UserRole).toBool();

        // If values are different, update the current item.
        if((*it) == mCurrentTreeWidgetItem)
        {
            if(val != value)
                (*it)->setData(index, Qt::UserRole, value);

            // Selected items are in bold
            QFont font = (*it)->font(WORKSPACE_NAME);
            font.setWeight(value ? QFont::Bold : QFont::Normal);
            (*it)->setFont(WORKSPACE_NAME, font);

            if(index == WORKSPACE_SELECTED)
            {
                if((*it)->isSelected() != value)
                {
                    (*it)->setSelected(value);

                    (*it)->setData(WORKSPACE_ACTIVE, Qt::UserRole, value);
                }
            }
            else if(index == WORKSPACE_MASK)
            {
                if(value)
                    (*it)->setIcon(WORKSPACE_STATUS, QIcon(":Src/FluoRender/UI/icons/misc/Mask.png"));
                else
                    (*it)->setIcon(WORKSPACE_STATUS, QIcon());
            }
        }
        // If values are different, update the other current items.
        else
        {
            bool active = ((*it)->text(WORKSPACE_NAME) == mCurrentViewWindow ||
                           (*it)->text(WORKSPACE_NAME) == mCurrentVolumeGroup ||
                           (*it)->text(WORKSPACE_NAME) == mCurrentVolume ||
                           (*it)->text(WORKSPACE_NAME) == mCurrentMeshGroup ||
                           (*it)->text(WORKSPACE_NAME) == mCurrentMesh);

            if(val != false)
                (*it)->setData(index, Qt::UserRole, false);

            // Active items are in bold
            QFont font = (*it)->font(WORKSPACE_NAME);
            font.setWeight(active ? QFont::Bold : QFont::Normal);
            (*it)->setFont(WORKSPACE_NAME, font);

            if(index == WORKSPACE_SELECTED)
            {
                if((*it)->isSelected())
                    (*it)->setSelected(false);

                (*it)->setData(WORKSPACE_ACTIVE, Qt::UserRole, active);
            }
            else if(index == WORKSPACE_MASK)
            {
                (*it)->setIcon(WORKSPACE_STATUS, QIcon());
            }
        }

        ++it;
    }
}

QString QvisDataManager::WorkspaceExists(QString name, DatasetAttributes::DatasetType dType)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << std::endl;

    int count = 0;
    QTreeWidgetItemIterator it(ui->WorkspaceTreeWidget);

    // Loop through all datasets of the same type and get the biggest index "_N" that might be
    // attached to the name.
    while(*it)
    {
        // Is it possible for a volume and mesh to have the same name???
        //if(DatasetAttributes::DatasetType((*it)->data(WORKSPACE_TYPE, Qt::UserRole).toInt()) == dType)
        {
            QString tmp = (*it)->text(WORKSPACE_NAME);

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

DatasetAttributes::AnimationSliderMode QvisDataManager::getAnimationSliderMode() const
{
    return mAnimationSliderMode;
}

void QvisDataManager::setAnimationSliderMode(DatasetAttributes::AnimationSliderMode value)
{
    mAnimationSliderMode = value;
}

int QvisDataManager::getAnimationPause() const
{
    return mAnimationPause;
}

void QvisDataManager::setAnimationPause(int value)
{
    mAnimationPause = value;
}

QIcon QvisDataManager::GetLightIcon(QColor color)
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

QIcon QvisDataManager::GetColorIcon(QColor color)
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

QList<QAction *> & QvisDataManager::getViewWindowActionList()
{
    return mViewWindowActionList;
}

QList<QAction *> & QvisDataManager::getVolumeActionList()
{
    return mVolumeActionList;
}

QList<QAction *> & QvisDataManager::getMeshActionList()
{
    return mMeshActionList;
}

