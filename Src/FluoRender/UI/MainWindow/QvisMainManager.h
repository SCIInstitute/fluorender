#ifndef QVISDATAMANAGER_H
#define QVISDATAMANAGER_H

#include <QWidget>
#include <QSettings>

#include <map>

class QListWidgetItem;
class QMenu;
class QSignalMapper;
class QSplitter;
class QTableWidgetItem;
class QTreeWidgetItem;

class DatasetAttributes
{
public:
    enum DatasetType {
        Unknown        = 0x0000,
        ActiveDatasets = 0x0001,
        ViewWindow     = 0x0002,
        VolumeGroup    = 0x0004,
        Volume         = 0x0008,
        MeshGroup      = 0x0010,
        Mesh           = 0x0020,
        Annotations    = 0x0040,
    };

    enum AnimationSliderMode {
        SingleStep = 0x0000,
        Continuous = 0x0001,
    };

    typedef std::pair<std::string, DatasetType> Dataset;
    typedef std::vector<Dataset> DatasetList;

    struct DataTreeNode
    {
        std::string name{""};
        DatasetType type{Unknown};
        double color[3] = {1,1,1};
        bool selected {false};
        bool visible{true};
        bool mask{false};
        std::vector< DataTreeNode > children;
    };

    void setDatasetList(DatasetList datasetList);
    DatasetList getDatasetList() const;

    void setDataTree(DataTreeNode dataTreeNode);
    DataTreeNode getDataTree() const;

private:
    DatasetList mDatasetList;
    DataTreeNode mDatatreeRoot;
};

namespace Ui {
class QvisDataManager;
}

// QvisAnimationWorker is a threaded worker for animating through a dataset.
// It is threaded so that the UI can continue to be active. Namely the user
// needs to be able to stop the animation.
class QvisAnimationWorker : public QObject
{
    Q_OBJECT

    friend class QvisDataManager;

public:
    QvisAnimationWorker();

signals:
    void finished();

public slots:
    void process();

protected:
    int  mAnimationPause{250};
};


// QvisDataManager
class QvisDataManager : public QWidget
{
    Q_OBJECT

    struct Selection
    {
        QString volumeGroup{""};
        QString meshGroup{""};
        std::map<QString, QString> datasets;
    };

public:
    explicit QvisDataManager(QWidget *parent = nullptr);
    ~QvisDataManager();

    void saveState(QSettings &Settings) const;
    void restoreState(const QSettings &Settings);

signals:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();

    void projectDirectoryChanged(QString directory);

    void actionViewWindowAdd();
    void actionViewWindowDelete(QString name);
    void actionVewWindowToggleView();
    void activeViewWindowChanged      (const QString name);
    void activeViewWindowVolumeChanged(const QString name, const QString volume);
    void activeViewWindowMeshChanged  (const QString name, const QString mesh);

    void updateViewWindow(const QString name);

public slots:
    // Sources
    void ProjectOpenClicked(QString filename = "");
    void ProjectSaveClicked();
    void ProjectSaveAsClicked();
    void VolumeOpenClicked(QString filename = "");
    void MeshOpenClicked(QString filename = "");

private slots:
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

    // Animation
    void AnimationReverseSingleStepClicked();
    void AnimationReverseClicked();
    void AnimationStopClicked();
    void AnimationPlayClicked();
    void AnimationForwardSingleStepClicked();
    void Animation();

    void AnimationSliderModeChanged(int);
    void AnimationPauseValueChanged(int);
    void AnimationIndexSliderReleased();
    void AnimationIndexSliderChanged(int);
    void AnimationIndexValueChanged(int);

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
public slots:
    void WorkspaceSelectionDeleteClicked(QString name = "");
    void WorkspaceViewWindowAddClicked(QString name = "");
private slots:
    QTreeWidgetItem * WorkspaceVolumeGroupAddClicked();
    QTreeWidgetItem * WorkspaceMeshGroupAddClicked();

public:
    DatasetAttributes::AnimationSliderMode getAnimationSliderMode() const;
    void setAnimationSliderMode(DatasetAttributes::AnimationSliderMode value);

    int  getAnimationPause() const;
    void setAnimationPause(int value);

    void WorkspaceViewWindowSelected(QString name);
    void WorkspaceViewWindowRename(QString currentName, QString newName);

    QList<QAction *> & getViewWindowActionList();
    QList<QAction *> & getVolumeActionList();
    QList<QAction *> & getMeshActionList();

private:
    Ui::QvisDataManager *ui;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

    void DatasetAdd(DatasetAttributes::DatasetType type, const QString &name, const QString &path);
    void DatasetUpdate(int index, bool value);

    void ViewWindowSelected (QTreeWidgetItem *item);
    void VolumeGroupSelected(QTreeWidgetItem *item);
    void MeshGroupSelected  (QTreeWidgetItem *item);
    void VolumeSelected     (QTreeWidgetItem *item);
    void MeshSelected       (QTreeWidgetItem *item);

    void ExpandParent(QTreeWidgetItem *item);
    void WorkspaceUpdate(int index, bool value);
    QString WorkspaceExists(QString name, DatasetAttributes::DatasetType dType);

    QTableWidgetItem * GetTableWidgetItem(const QString &name, bool exact = true) const;
    QTreeWidgetItem  * GetTreeWidgetItem (const QString &name, bool exact = true) const;
    QTreeWidgetItem  * GetTreeWidgetItemViewWindow(const QString &name) const;

    QMenu * GetTreeWidgetMenu(QWidget *parent, const DatasetAttributes::DatasetType datasetType);
    void    GetTreeWidgetMenu(const DatasetAttributes::DatasetType datasetType,
                              const QTreeWidgetItem *item,
                              QMenu *menu,
                              QSignalMapper* signalMapper) const;

    QIcon GetColorIcon(QColor color = QColor(255,255,255));
    QIcon GetLightIcon(QColor color = QColor(255,255,255));

    int mCurrentDataSetIndex{-1};
    QString mCurrentDataSet{""};

    int mNumVolumeGroups{0};
    int mNumMeshGroups{0};

    QString mCurrentViewWindow{""};
    QString mCurrentVolumeGroup{""};
    QString mCurrentMeshGroup{""};
    QString mCurrentVolume{""};
    QString mCurrentMesh{""};

    std::map<QString, Selection> mLastSelections;

    QTreeWidgetItem *mCurrentTreeWidgetItem{nullptr};

private:
    QSplitter *mTopSplitter{nullptr};
    QSplitter *mMiddleSplitter{nullptr};
    QSplitter *mBottomSplitter{nullptr};

    QList<QAction *> mViewWindowActionList;
    QList<QAction *> mVolumeActionList;
    QList<QAction *> mMeshActionList;

    // Used when saving/restoring state.
    bool mRestoringState {false};
    int  mCurrentVersion {1};

    // For animation
    DatasetAttributes::AnimationSliderMode mAnimationSliderMode{DatasetAttributes::SingleStep};
    int  mAnimationPause{250};

    bool mAnimationContinuous{false};
    bool mAnimationStop{true};
    int  mAnimationStep{0};

    // Project management.
    bool haveProject{false};
};

#endif // QVISDATAMANAGER_H
