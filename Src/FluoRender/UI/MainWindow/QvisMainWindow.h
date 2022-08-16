#ifndef QVISMAINWINDOW_H
#define QVISMAINWINDOW_H

#include <QMainWindow>

#include <QEvent>
#include <QMap>

#include "fluorender-config.h"

#ifdef HAVE_ADS
namespace ads {
class CDockManager;
class CDockWidget;
}
#else
class QSplitter;
#endif

namespace Ui {
class QvisMainWindow;
}

class QvisDialogBase;
class QvisDragDropToolBar;
class QvisMessageDialog;

class QvisDataManager;
class QvisViewWindow;

// Preferences
class QvisPreferencesDialog;
class QvisCapturePreferencesDialog;
class QvisMoviePreferencesDialog;

class CapturePreferencesAttributes;
class MoviePreferencesAttributes;

// Properties
class QvisVolumePropertiesDialog;
class QvisMeshPropertiesDialog;
class QvisViewPropertiesDialog;
class QvisOutputImagePropertiesDialog;

// Tools
class QvisCalculationDialog;
class QvisClipDialog;
class QvisColocalizationDialog;
class QvisComponentAnalyzerDialog;
class QvisConvertDialog;
class QvisExportDialog;
class QvisMeasurementDialog;
class QvisNoiseReductionDialog;
class QvisPaintBrushDialog;
class QvisOpenCLKernelEditorDialog;
class QvisTrackingDialog;
class QvisVolumeSizeDialog;

// Mesh
class QvisMeshTransformDialog;

class QComboBox;

class QvisMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QvisMainWindow(QWidget *parent = nullptr);
    ~QvisMainWindow();

    void loadFile(QString filename);

    virtual void closeEvent(QCloseEvent* event) override;

private slots:
    void onFocusWindowChanged(QWindow* window);

    // Layout menu
    void ActionSaveLayoutState();
    void ActionRestoreLayoutState();

    // Perspective menu
    void ActionCreatePerspective(bool needName = true);
    void ActionDeletePerspective(QAction *action);
    void ActionDeleteAllPerspectives();
    void ActionSetPerspective(QAction *action);

    // General
    void ActionAbout();
    void ActionPreferences();
    void ActionCapturePreferences();
    void ActionMoviePreferences();

    // View
    void ActionViewWindowNew(QvisViewWindow *viewWindow = nullptr);
    void ActionViewWindowDelete(QString name);
    void ActionViewWindowToggleView();

    // Help
    void ActionFluoRenderWebsite();
    void ActionCheckForUpdates();
    void ActionDownloadManual();
    void ActionDownloadTutorial();

    // Misc
    void ProjectDirectoryChanged(QString directory);

    // View Window Management
#ifdef HAVE_ADS
    void FocusedDockWidgetChanged(ads::CDockWidget* previous, ads::CDockWidget* current);
#endif
    void UpdateViewWindow(QString name);

    void ActiveViewWindowChanged(QString name, QEvent::Type type = QEvent::WindowActivate);
    void ActiveViewWindowVolumeChanged(QString name, QString volume);
    void ActiveViewWindowMeshChanged  (QString name, QString mesh);
    void UpdateViewWindowMenus(QString name, bool connect) const;

private:
    // Perspective action - Helpers.
    void SavePerspective();
    void RestorePerspectives();
    void PopulatePerspectiveMenus();

    void UpdateViewWindowSignals(bool connect) const;
    void UpdateViewWindowList();

    void ConnectToMessageDialog(QWidget *widget) const;

    void AddDialog(QvisDialogBase *dialog, QMenu *menu, QvisDragDropToolBar *toolbar, QList<QAction *> *actionList = nullptr);

    Ui::QvisMainWindow *ui{nullptr};

#ifdef HAVE_ADS
    ads::CDockManager *mDockManager{nullptr};

    QComboBox *mPerspectiveComboBox{nullptr};
    QToolBar  *mDockLayoutToolbar{nullptr};
#else
    QSplitter *mMainSplitter;
#endif

    std::vector<QvisDialogBase *> mDialogWidgets;

    // Main Windows
    QvisDataManager           *mDataManager{nullptr};
    QvisViewWindow            *mViewWindow{nullptr};
    QvisMessageDialog         *mMessageDialog{nullptr};

    std::map<QString, QvisViewWindow *> mViewWindowMap;

    // Preferences
    QvisPreferencesDialog           *mPreferencesDialog{nullptr};
    QvisCapturePreferencesDialog    *mCapturePreferencesDialog{nullptr};
    QvisMoviePreferencesDialog      *mMoviePreferencesDialog{nullptr};

    // Properties
    QvisVolumePropertiesDialog      *mVolumePropertiesDialog{nullptr};
    QvisMeshPropertiesDialog        *mMeshPropertiesDialog{nullptr};
    QvisViewPropertiesDialog        *mViewPropertiesDialog{nullptr};
    QvisOutputImagePropertiesDialog *mOutputImagePropertiesDialog{nullptr};

    // Tools
    QvisCalculationDialog        *mCalculationDialog{nullptr};
    QvisClipDialog               *mClipDialog{nullptr};
    QvisColocalizationDialog     *mColocalizationDialog{nullptr};
    QvisComponentAnalyzerDialog  *mComponentAnalyzerDialog{nullptr};
    QvisConvertDialog            *mConvertDialog{nullptr};
    QvisExportDialog             *mExportDialog{nullptr};
    QvisMeasurementDialog        *mMeasurementDialog{nullptr};
    QvisNoiseReductionDialog     *mNoiseReductionDialog{nullptr};
    QvisOpenCLKernelEditorDialog *mOpenCLKernelEditorDialog{nullptr};
    QvisPaintBrushDialog         *mPaintBrushDialog{nullptr};
    QvisTrackingDialog           *mTrackingDialog{nullptr};
    QvisVolumeSizeDialog         *mVolumeSizeDialog{nullptr};

    // Mesh
    QvisMeshTransformDialog      *mMeshTransformDialog{nullptr};

    // Toolbar widgets
    QvisDragDropToolBar *mToolsToolbar{nullptr};
    QvisDragDropToolBar *mPropertiesToolbar{nullptr};

    // Misc
    CapturePreferencesAttributes *mCapturePreferencesAttributes{nullptr};
    MoviePreferencesAttributes   *mMoviePreferencesAttributes{nullptr};
};

#endif // QVISMAINWINDOW_H
