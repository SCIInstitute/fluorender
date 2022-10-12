#ifndef QVISWORKSPACEMANAGER_H
#define QVISWORKSPACEMANAGER_H

#include "QvisInterfaceBase.h"
#include "DatasetAttributes.h"

#ifdef QT_CREATOR_ONLY
  class WorkspaceAgent;
#else
  namespace fluo {
    class WorkspaceAgent;
  }
  using namespace fluo;
#endif

#include <QWidget>

#include <map>

class QvisDatasetManager;

class QMenu;
class QSignalMapper;
class QTreeWidgetItem;

class DataTreeNode;

namespace Ui {
class QvisWorkspaceManager;
}

// QvisWorkspaceManager
class QvisWorkspaceManager : public QWidget, public QvisInterfaceBase
{
    Q_OBJECT
public:
    explicit QvisWorkspaceManager(QWidget *parent = nullptr);
    ~QvisWorkspaceManager();

    virtual void SetAgent(InterfaceAgent* agent) override;
    virtual void updateWindow(bool doAll = true) override;

    void setDatasetManager(QvisDatasetManager *datasetManager);

    void WorkspaceViewWindowSelected(QString name);
    void WorkspaceViewWindowRename(QString currentName, QString newName);

    QMenu * GetTreeWidgetMenu(QMenu *parent, const DatasetAttributes::DatasetType datasetType);

    QList<QAction *> & getViewWindowActionList();
    QList<QAction *> & getVolumeActionList();
    QList<QAction *> & getMeshActionList();

signals:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();

    void actionViewWindowAdd();
    void actionViewWindowDelete(QString name);
    void actionVewWindowToggleView();
    void activeViewWindowChanged      (const QString name);
    void activeViewWindowVolumeChanged(const QString name, const QString volume);
    void activeViewWindowMeshChanged  (const QString name, const QString mesh);

    void updateViewWindow(const QString name);

    void datasetUpdateSelection(const QString name, bool exact);

public slots:
    void WorkspaceUpdateSelection(const QString selName, DatasetAttributes::DatasetType datasetType);
    void WorkspaceDelete         (const QString delName, DatasetAttributes::DatasetType datasetType, const QString fullname);
    void WorkspaceRename         (const QString oldName,    const QString newName, DatasetAttributes::DatasetType datasetType, const QString fullname);
    void WorkspaceAdd            (const QString parentName, const QString addname, DatasetAttributes::DatasetType datasetType, const QString fullname);

    void WorkspaceSelectionDeleteClicked(QString name = "");
    void WorkspaceViewWindowAddClicked(QString name = "");

private slots:
    // Workspace view
    void WorkspaceSelected     (QTreeWidgetItem *item, int column);
    void WorkspaceIconClicked  (QTreeWidgetItem *item, int column);
    void WorkspaceDoubleClicked(QTreeWidgetItem *item, int column);
    void WorkspaceChanged      (QTreeWidgetItem *item, int column);
    void WorkspaceItemMoved    (QTreeWidgetItem *item,   int iColumn,
                                QTreeWidgetItem *parent, int pColumn);
    void WorkspaceContextMenuRequested(const QPoint &pos);

    void WorkspaceToggleTreeWidgetItem();
    void WorkspaceShowAll(QTreeWidgetItem *item = nullptr);
    void WorkspaceIsolate();
    void WorkspaceRandomizeColors(QTreeWidgetItem *item = nullptr);
    void WorkspaceRename();
    void ViewWindowToggleView();
    void VolumeCopyMask();
    void VolumePasteMask();
    void VolumeMergeMask();
    void VolumeExcludeMask();
    void VolumeIntersectMask();

    // Workspace buttons
    void WorkspaceHideShowClicked();

    QTreeWidgetItem * WorkspaceGroupAddClicked(DatasetAttributes::DatasetType dType);

private:
    // Base names for the tree view groups.
    const QString ACTIVE_DATASETS_STR {"Active Datasets"};
    const QString VIEW_WINDOW_STR     {"View Window "};
    const QString VOLUME_GROUP_STR    {"Volume Group "};
    const QString MESH_GROUP_STR      {"Mesh Group "};

    // Used with the workspace tree widget which has multiple columns;
    // name and a status icon.
    enum ItemColumn
    {
        ITEM_NAME   = 0,
        ITEM_COLOR  = 1,
        ITEM_STATUS = 2,
    };

    // Used with the QTreeWidgetItem for the user data in each item
    enum ItemDataIndex
    {
        ITEM_DATA_TYPE = 0,
        ITEM_DATA_NODE = 1,
        ITEM_DATA_HASH = 2,
    };

    // Used with the Node data to set get data.
    enum NodeDataIndex
    {
        NODE_DATA_NAME     = 0,
        NODE_DATA_TYPE     = 1,
        NODE_DATA_COLOR    = 2,
        NODE_DATA_SELECTED = 3,
        NODE_DATA_ACTIVE   = 4,
        NODE_DATA_VISIBILE = 5,
        NODE_DATA_MASK     = 6,
        NODE_DATA_FULLNAME = 7,
    };

    struct Selection
    {
        QString volumeGroup{""};
        QString meshGroup{""};
        std::map<QString, QString> datasets;
    };

private:
    Ui::QvisWorkspaceManager *ui;

    bool addChildItem(QTreeWidgetItem * praentItem, const DataTreeNode *childNode, bool createItem);
    QTreeWidgetItem * WorkspaceActiveDatasetsAdd(DataTreeNode *node = nullptr);

    void ViewWindowSelected (QTreeWidgetItem *item);
    void VolumeGroupSelected(QTreeWidgetItem *item);
    void MeshGroupSelected  (QTreeWidgetItem *item);
    void VolumeSelected     (QTreeWidgetItem *item);
    void MeshSelected       (QTreeWidgetItem *item);

    void ExpandParent(QTreeWidgetItem *item);
    void WorkspaceUpdate(NodeDataIndex index, bool value);
    QString WorkspaceExists(QString name, DatasetAttributes::DatasetType dType);

    QTreeWidgetItem  * GetTreeWidgetItem (const QString &name, bool exact = true) const;
    QTreeWidgetItem  * GetTreeWidgetItemViewWindow(const QString &name) const;

    void GetTreeWidgetMenu(const DatasetAttributes::DatasetType datasetType,
                           const QTreeWidgetItem *item,
                           QMenu *menu,
                           QSignalMapper* signalMapper) const;

    QIcon GetColorIcon(QColor color = QColor(255,255,255));
    QIcon GetLightIcon(QColor color = QColor(255,255,255));

    // Helper methods.
    inline void setItemNode(QTreeWidgetItem *item, const DataTreeNode * node);
    inline DataTreeNode * getItemNode(QTreeWidgetItem *item);

    inline void setItemType(QTreeWidgetItem *item, const DatasetAttributes::DatasetType);
    inline DatasetAttributes::DatasetType getItemType(const QTreeWidgetItem *item) const;

    inline void     setNodeData(QTreeWidgetItem *item, NodeDataIndex index, QVariant value);
    inline QVariant getNodeData(QTreeWidgetItem *item, NodeDataIndex index);

    inline void     setNodeData(DataTreeNode *item, NodeDataIndex index, QVariant value);
    inline QVariant getNodeData(DataTreeNode *item, NodeDataIndex index);

    WorkspaceAgent* mAgent {nullptr};

    int mNumVolumeGroups{0};
    int mNumMeshGroups{0};

    QString mCurrentViewWindow{""};
    QString mCurrentVolumeGroup{""};
    QString mCurrentMeshGroup{""};
    QString mCurrentVolume{""};
    QString mCurrentMesh{""};

    std::map<QString, Selection> mLastSelections;

    QTreeWidgetItem *mCurrentTreeWidgetItem{nullptr};

    QList<QAction *> mViewWindowActionList;
    QList<QAction *> mVolumeActionList;
    QList<QAction *> mMeshActionList;

    QvisDatasetManager *mDatasetManager {nullptr};
};

#endif // QVISWORKSPACEMANAGER_H
