#ifndef QVISDATASETMANAGER_H
#define QVISDATASETMANAGER_H

#include "QvisInterfaceBase.h"
#include "DatasetAttributes.h"

#include <QFileInfo>
#include <QWidget>

class QTableWidgetItem;
class QvisWorkspaceManager;

#ifdef QT_CREATOR_ONLY
class DatasetAgent;
#else
  namespace fluo {
    class DatasetAgent;
  }
  using namespace fluo;
#endif

namespace Ui {
class QvisDatasetManager;
}

// QvisDatasetManager
class QvisDatasetManager : public QWidget, public QvisInterfaceBase
{
    Q_OBJECT

public:
    explicit QvisDatasetManager(QWidget *parent = nullptr);
    ~QvisDatasetManager();

    virtual void SetAgent(InterfaceAgent* agent) override;
    virtual void updateWindow(bool doAll = true) override;

    void setWorkspaceManager(QvisWorkspaceManager *workspaceManager);

signals:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();

    void workspaceUpdateSelection(const QString selName, DatasetAttributes::DatasetType datasetType);
    void workspaceDelete         (const QString delName, DatasetAttributes::DatasetType datasetType, const QString fullname);
    void workspaceRename         (const QString oldName,    const QString newName, DatasetAttributes::DatasetType datasetType, const QString fullname);
    void workspaceAdd            (const QString parentName, const QString addName, DatasetAttributes::DatasetType datasetType, const QString fullname);

public slots:
    // Dataset view
    void DatasetUpdateSelection(const QString name, bool selected);

//private slots:
    // Dataset view
    void DatasetSelected(int row, int column);
    void DatasetDoubleClicked(QTableWidgetItem *item);
    void DatasetChanged(int row, int column);
    void DatasetContextMenuRequested(const QPoint &pos);

    // Dataset buttons
    void DatasetAddClicked(const QString name = "");
    void DatasetRenameClicked();
    void DatasetSaveAsClicked();
    void DatasetDeleteClicked();
    void DatasetDeleteAllClicked();
    void DatasetApplyPropertiesClicked();
    void DatasetSaveMaskClicked();

private:
    // Used with the DatasetTableWidget which has multiple columns;
    enum ItemColumnIndex
    {
        DATASET_TYPE = 0,
        DATASET_NAME = 1,
        DATASET_PATH = 2,
    };

    Ui::QvisDatasetManager *ui;

    QTableWidgetItem * GetTableWidgetItem(const QString &name, bool exact = true) const;
    DatasetAttributes::DatasetType getDatasetType(const int index) const;
    QString getDatasetName(const int index) const;
    QString getDatasetFullName(const int index) const;

    void DatasetAdd(const QString &fullname, DatasetAttributes::DatasetType type);
    void DatasetUpdate(int index, bool value);

    DatasetAgent* mAgent {nullptr};

    int     mCurrentDataSetIndex{-1};
    QString mCurrentDataSetName{""};

    // For menus
    QvisWorkspaceManager *mWorkspaceManager{nullptr};
};

#endif // QVISDATASETMANAGER_H
