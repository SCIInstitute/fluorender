#include "QvisDatasetManager.h"
#include "ui_QvisDatasetManager.h"

#include "QtCore/qpoint.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "QvisWorkspaceManager.h"

#ifdef QT_CREATOR_ONLY
  #include "DatasetAgent.h"
#else
  #include "DatasetAgent.hpp"
  using namespace fluo;
#endif

#include <filesystem>
#include <iostream>

// Used with the dataset table widget which has multiple columns;
#define DATASET_TYPE 0
#define DATASET_NAME 1
#define DATASET_PATH 2

// QvisDatasetManager
QvisDatasetManager::QvisDatasetManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisDatasetManager)
{
    ui->setupUi(this);

    // Dummy entries.
    mAgent = new DatasetAgent();

    DatasetAdd(QString("/meshpath/Some_Mesh.obj"),     DatasetAttributes::Mesh);
    DatasetAdd(QString("/volumepath/Some_Volume.png"), DatasetAttributes::Volume);
}

QvisDatasetManager::~QvisDatasetManager()
{
    delete ui;
}

void QvisDatasetManager::SetAgent(InterfaceAgent* agent)
{
    mAgent = (DatasetAgent *) agent;
}

void QvisDatasetManager::updateWindow(bool doAll)
{
    const DatasetAgent::DatasetList &datasetList = mAgent->getDatasetList();

    int currentIndex = -1;
    int cc = 0;

    for(const auto & dataset : datasetList)
    {
        std::string fullname = dataset.first;
        QFileInfo fileInfo(fullname.c_str());
        std::string name = fileInfo.baseName().toStdString();

        // The currently selected uses the base name.
        if(mAgent->getActiveDataset() == name)
        {
            currentIndex = cc;
            break;
        }

        ++cc;
    }

    // Update the dataset table.
    if(doAll)
    {
        const QSignalBlocker blocker(ui->DatasetTableWidget);
        // Delete the table row by row so to also delete the tree.
        while(ui->DatasetTableWidget->rowCount())
        {
            int i = 0;
            ui->DatasetTableWidget->removeRow(i);
        }

        for(const auto & dataset: datasetList)
        {
            std::string fullname = dataset.first;
            DatasetAttributes::DatasetType type = dataset.second;

            DatasetAdd(fullname.c_str(), type);
        }
    }

    // Update the dataset selected
    DatasetSelected(currentIndex, 0);
}

void QvisDatasetManager::setWorkspaceManager(QvisWorkspaceManager *workspaceManager)
{
    mWorkspaceManager = workspaceManager;
}

// Dataset view

// Signal from the workspace when a dataset is selected
void QvisDatasetManager::DatasetUpdateSelection(const QString name, bool selected)
{
    QTableWidgetItem * tableItem = GetTableWidgetItem(name);

    if(tableItem != nullptr)
    {
        mCurrentDataSetName = tableItem->text();
        mCurrentDataSetIndex = tableItem->row();
    }
    // This condition happens when the dataset selected is a "copy" of the original.
    else
    {
        mCurrentDataSetName = "";
        mCurrentDataSetIndex = -1;
    }

    DatasetUpdate(mCurrentDataSetIndex, selected);
}

void QvisDatasetManager::DatasetSelected(int row, int column)
{
    Q_UNUSED(column);

    // The selection is the whole row so the column is unused.
    mCurrentDataSetIndex = row;

    DatasetAttributes::DatasetType datasetType;

    if(mCurrentDataSetIndex != -1)
    {
        QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);
        mCurrentDataSetName = tableItem->text();
        datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                    DatasetAttributes::Volume : DatasetAttributes::Mesh;
    }
    else
    {
        mCurrentDataSetName = "";
        datasetType = DatasetAttributes::Unknown;
    }

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << mCurrentDataSetName.toStdString() << "'  " << datasetType << std::endl;

    // Only allow one dataset to be selected.
    DatasetUpdate(mCurrentDataSetIndex, true);

    // When selecting a dataset, if present in the workspace tree select it too.
    emit workspaceUpdateSelection(mCurrentDataSetName, datasetType);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << mCurrentDataSetName.toStdString() << "'  " << datasetType << std::endl;
}

void QvisDatasetManager::DatasetDoubleClicked(QTableWidgetItem *item)
{
    if(item)
        DatasetSelected(item->row(), 0);
}

void QvisDatasetManager::DatasetChanged(int row, int column)
{
    // The only time a dataset changes is if the name is edited.
    if(column != DATASET_NAME)
        return;

    QTableWidgetItem *item = ui->DatasetTableWidget->item(row, DATASET_NAME);
    QString newName = item->text(); // New name

    if(newName == mCurrentDataSetName)
        return;

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentDataSetName.toStdString() << "  " << newName.toStdString() << std::endl;

    // Make sure the new dataset name is different than other dataset names.
    // Check without any whitespace.
    QString tmpName = newName.simplified(); // New name with whitespace cleanup.
    tmpName.remove(" "); // Remove all whitespace;

    for(int i=0; i<ui->DatasetTableWidget->rowCount(); ++i)
    {
        QString tmp = ui->DatasetTableWidget->item(i, DATASET_NAME)->text().simplified();
        tmp.remove(" "); // Remove all whitespace;

        if(i != row && tmpName == tmp)
        {
            const QSignalBlocker blocker(ui->DatasetTableWidget);
            item->setText(mCurrentDataSetName);

            QString message("Cannot rename the dataset because the name is already in use.");
            emit postErrorMessage(message);

            return;
        }
    }

    item = ui->DatasetTableWidget->item(row, DATASET_TYPE);

    DatasetAttributes::DatasetType datasetType = item->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    // Get the original full name which is unique so to diambiguate as the local name could be different.
    QString fullname = item->data(Qt::UserRole).toString();

    emit workspaceRename(mCurrentDataSetName, newName, datasetType, fullname);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mCurrentDataSetName.toStdString() << "  " << newName.toStdString() << std::endl;

    mCurrentDataSetIndex = row;
    mCurrentDataSetName = newName;

    DatasetUpdate(mCurrentDataSetIndex, true);
}

void QvisDatasetManager::DatasetContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = ui->DatasetTableWidget->itemAt(pos);

    if(item)
    {
        mCurrentDataSetIndex = item->row();
        item = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);
        mCurrentDataSetName = item->text();

        DatasetUpdate(mCurrentDataSetIndex, true);

        DatasetAttributes::DatasetType datasetType = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE)->text() == "Volume" ?
                    DatasetAttributes::Volume : DatasetAttributes::Mesh;

        QMenu *contextMenu = new QMenu(this);

        // Add the currently selected view window and if present volume and mesh groups.
        QMenu *viewWindowMenu = mWorkspaceManager->GetTreeWidgetMenu(contextMenu, datasetType);
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
void QvisDatasetManager::DatasetAddClicked(const QString parentName)
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.size() == 0)
    {
        QString message("Cannot add a dataset because a dataset is not selected.");
        emit postErrorMessage(message);

        return;
    }

    QTableWidgetItem *item = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE);

    DatasetAttributes::DatasetType datasetType = item->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    // Get the original full name which is unique so to diambiguate as the local name could be different.
    QString fullname = item->data(Qt::UserRole).toString();

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << parentName.toStdString() << "'  "  << "  '" << mCurrentDataSetName.toStdString() << "'  " << datasetType << std::endl;

    // If a parentName then the signal is from a menu action so use the name to get the parent group.
    emit workspaceAdd(parentName, mCurrentDataSetName, datasetType, fullname);
}

void QvisDatasetManager::DatasetRenameClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.size() == 0)
    {
        QString message("Cannot rename a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }

    QTableWidgetItem *item = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME);

    if(item)
        ui->DatasetTableWidget->editItem(item);
}

void QvisDatasetManager::DatasetSaveAsClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.size() == 0)
    {
        QString message("Cannot save a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDatasetManager::DatasetDeleteClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.isEmpty())
    {
        QString message("Cannot delete a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }

    QTableWidgetItem *item = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_TYPE);

    DatasetAttributes::DatasetType datasetType = item->text() == "Volume" ?
                DatasetAttributes::Volume : DatasetAttributes::Mesh;

    // Get the original full name which is unique so to diambiguate as the local name could be different.
    QString fullname = item->data(Qt::UserRole).toString();

    // Delete the dataset from the table.
    const QSignalBlocker blocker(ui->DatasetTableWidget);
    ui->DatasetTableWidget->removeRow(mCurrentDataSetIndex);

    // Delete the dataset from the workspace.
    emit workspaceDelete(mCurrentDataSetName, datasetType, fullname);

    mCurrentDataSetIndex = -1;
    mCurrentDataSetName = tr("");

    // After deleting an entry a new one may automatically be selected so update the current dataset.
    int cc = 0;
    while(ui->DatasetTableWidget->rowCount())
    {
        if(ui->DatasetTableWidget->item(cc, DATASET_NAME)->isSelected())
        {
            mCurrentDataSetIndex = cc;
            mCurrentDataSetName = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME)->text();

            break;
        }
        else
        {
            ++cc;
        }
    }

    DatasetSelected(mCurrentDataSetIndex, true);
}

void QvisDatasetManager::DatasetDeleteAllClicked()
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
        mCurrentDataSetName = ui->DatasetTableWidget->item(mCurrentDataSetIndex, DATASET_NAME)->text();

        DatasetDeleteClicked();
    }

    mCurrentDataSetIndex = -1;
    mCurrentDataSetName = tr("");

    DatasetSelected(mCurrentDataSetIndex, false);
}

void QvisDatasetManager::DatasetApplyPropertiesClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.size() == 0)
    {
        QString message("Cannot apply properties to a dataset because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDatasetManager::DatasetSaveMaskClicked()
{
    if(mCurrentDataSetIndex < 0 || mCurrentDataSetName.size() == 0)
    {
        QString message("Cannot save a dataset mask because no dataset is selected.");
        emit postErrorMessage(message);

        return;
    }
}

void QvisDatasetManager::DatasetAdd(const QString &fullname, DatasetAttributes::DatasetType type)
{
    QFileInfo fileInfo(fullname);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << fileInfo.baseName().toStdString() << "'  " << type << std::endl;

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

    // Keep the original fullname for the entry so that when deleting
    // those with different names they are deleted too.
    item->setData(Qt::UserRole, QVariant(fullname));

    if(item)
    {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 0, item);

        item = new QTableWidgetItem(fileInfo.baseName());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 1, item);

        item = new QTableWidgetItem(fileInfo.absolutePath());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->DatasetTableWidget->setItem(row, 2, item);
    }
}

// Update the dataset that is to be selected. Only one can be selected.
void QvisDatasetManager::DatasetUpdate(int index, bool selected)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << index << "  " << selected << std::endl;

    if(!selected || index == -1)
        mAgent->setActiveDataset("");
    else if(selected)
    {
        // The currently selected uses the base name.
        if(mAgent->getActiveDataset() != mCurrentDataSetName.toStdString())
            mAgent->setActiveDataset(mCurrentDataSetName.toStdString());
    }

    const QSignalBlocker blocker(ui->DatasetTableWidget);

    for(int row=0; row<ui->DatasetTableWidget->rowCount(); ++row)
    {
        for(int col=0; col<ui->DatasetTableWidget->columnCount(); ++col)
        {
            QTableWidgetItem *tableItem = ui->DatasetTableWidget->item(row, col);

            if(row == index)
            {
                if(tableItem->isSelected() != selected)
                    tableItem->setSelected(selected);
            }
            else
            {
                if(tableItem->isSelected() != false)
                    tableItem->setSelected(false);
            }
        }
    }
}

QTableWidgetItem * QvisDatasetManager::GetTableWidgetItem(const QString &name, bool exact) const
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
