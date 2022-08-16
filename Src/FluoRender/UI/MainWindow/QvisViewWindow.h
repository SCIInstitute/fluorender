#ifndef QVISVIEWWINDOW_H
#define QVISVIEWWINDOW_H

#include <QEvent>
#include <QLabel>
#include <QSettings>
#include <QSplitter>
#include <QWidget>

#include <QvisCapturePreferencesDialog.h>

class QContextMenuEvent;

class QvisDragDropToolBar;
class QvisOpenGLWidget;
class QvisViewWindowDialog;


namespace Ui {
class QvisViewWindow;
}

class QvisViewWindow : public QWidget
{
    Q_OBJECT

    friend class QvisMainWindow;

    // must match the mViewModes below.
    enum LayoutMode
    {
        LayoutSingle     = 0,
        LayoutVertical   = 1,
        LayoutHorizontal = 2,
        LayoutQuad       = 3,
        LayoutUnknown,
    };

public:
    explicit QvisViewWindow(QWidget *parent = nullptr);
    explicit QvisViewWindow(int id, QWidget *parent = nullptr);
    ~QvisViewWindow();

    void saveState(QSettings &Settings) const;
    void restoreState(const QSettings &Settings);

    QvisViewWindowDialog * getViewWindowDialog() const;
    void setViewWindowDialog(QvisViewWindowDialog *dialog);

    bool hasDragDropToolBar() const;
    QvisDragDropToolBar * dragDropToolBar();

    void setActiveVolume(QString name);
    void setActiveMesh(QString name);

    int activeSubWindowIndex() const;
    LayoutMode layoutMode() const;

    void setCapturePreferencesAttributes(CapturePreferencesAttributes *atts);

    void setViewWindowActionList(QList<QAction *> & list);

    void update();

signals:
    void postInformationalMessage(const QString message);
    void postWarningMessage      (const QString message);
    void postErrorMessage        (const QString message);
    void clearMessage();

    void activeViewWindowChanged (const QString name, const QEvent::Type);
    void activeWindowIndexChanged(const int index);
    void updateViewWindowMenus   (const QString name, const bool connect);

public slots:
    // Used privately but also called by QvisMainWindow publically
    void LayoutModeSingle();
    void LayoutModeHorizontal();
    void LayoutModeVertical();
    void LayoutModeQuad();

    void CurrentCursorMode(QString tool, QString mode);

private slots:
    void ViewModeIndexChanged(int index);
    void SplitterModeIndexChanged(int index);
    void SplitterMoved(int pos, int index);

    void InitializeWindow(const QString name);
    void PaintWindow(const QString name);
    void ResizeWindow(const QString name, const int w, const int h);

    void ActiveSubWindow(const QString name);
    void MousePress  (const QPointF &point);
    void MouseRelease(const QPointF &point);
    void MouseMove   (const QPointF &point);

    void ContextMenuRequested(const QPoint &pos);

    void CaptureButtonClicked();

    void EnterFullScreenMode();
    void ExitFullScreenMode();

private:
    Ui::QvisViewWindow *ui;

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    void setup();
    void updateID(int id);

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);
    void ViewModeUpdate();

    enum SplitterMode
    {
        Fixed   = 0,
        Movable = 1,
    };

    const QStringList mSplitterModes {"Fixed", "Movable"};

    // The view for each layout, for all layouts the first sub window is ALWAYS 3D.
    // As such, the view mode indicates the mode for the other windows. Which are axis aligned
    // views. View_XYZ is used only when there all four sub windows are displayed as to indicate
    // all are axis aligned (opposed to all being 3D).

    // Also used for the view mode of each sub window (View_XYZ is meaningless).
    enum ViewMode
    {
        View_X,
        View_Y,
        View_Z,
        View_3D,
        View_XYZ,
    };

    static constexpr int mNumLayoutModes {4};

    // The possible sub window view modes which is dependent on the layout mode.
    const QStringList mViewModes[mNumLayoutModes] {{   "X Axis"    ,    "Y Axis",    "Z Axis",     "3D"},
                                                   {"3D-X Axis"    , "3D-Y Axis", "3D-Z Axis", "All 3D"},
                                                   {"3D-X Axis"    , "3D-Y Axis", "3D-Z Axis", "All 3D"},
                                                   {"3D-X-Y-Z Axis", "All 3D"}};

    const std::vector<int> mViewLayoutWindows[mNumLayoutModes] {{0},
                                                                {0, 1},
                                                                {0, 2},
                                                                {0, 1, 2, 3}};

    const QString mViewLayoutExtension[mNumLayoutModes] {"a", "b", "c", "d"};

    static constexpr int mNumSubWindows {4};

    QvisOpenGLWidget *mOpenGLWidget[mNumSubWindows] {nullptr, nullptr, nullptr, nullptr};

    QvisDragDropToolBar * mDragDropToolbar{nullptr};

    // The view window's unique id.
    int mID {-1};

    int mActiveSubWindowIndex     {-1}; // The sub window that is currently active.
    int mContextSubWindowIndex    {-1}; // The sub window active when the context menu was created.
    int mFullScreenSubWindowIndex {-1}; // The sub window current shown full screen.

    // Used when saving/restoring state.
    bool mRestoringState {false};
    int  mCurrentVersion {1};

    // The parent view window dialog that contains this view window. Used when
    // NOT compiled with the ADS and for full screen mode of a single OpenGl Window.
    QvisViewWindowDialog *mViewWindowDialog {nullptr};

    // The current view layout
    LayoutMode    mLayoutMode  {LayoutSingle};
    // The current splitter state
    SplitterMode mSplitterMode {Movable};
    // Current view mode for each of the view layouts.
    ViewMode mViewMode[mNumLayoutModes]         {View_3D, View_3D, View_3D, View_XYZ};
    // Current view mode (3D or axis aligned) for each of the sub windows.
    ViewMode mSubWindowViewMode[mNumSubWindows] {View_3D, View_3D, View_3D, View_3D};

    bool mHideModeGroupBox {false};

    // Temporary storage for the splitter state when going between adjustable and fixed
    // window size.
    QByteArray mMainSplitterData;
    QByteArray mTopSplitterData;
    QByteArray mBottomSplitterData;

    QList<QAction *> mViewWindowActionList;

    CapturePreferencesAttributes * mCapturePreferencesAttributes{nullptr};

    // A counter so tht each view window gets a unique id.
    static int mNumViewWindows;
};

#endif // QVISVIEWWINDOW_H
