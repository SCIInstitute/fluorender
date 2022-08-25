#include "QvisMainWindow.h"
#include "ui_QvisMainWindow.h"

#include "QtGui/qwindow.h"

#include <QDesktopServices>
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QWidgetAction>

// Advanced docking system.
#ifdef HAVE_ADS
  #include "DockManager.h"
  #include "DockAreaWidget.h"
  #include "DockSplitter.h"
#endif

// Toolbars
#include "QvisDragDropToolBar.h"

// Base windows.
#include "QvisMainManager.h"
#include "QvisSourceManager.h"
#include "QvisDatasetManager.h"
#include "QvisWorkspaceManager.h"
#include "QvisAnimationController.h"
#include "QvisMessageDialog.h"
#include "QvisViewWindow.h"
#ifndef HAVE_ADS
  #include "QvisViewWindowDialog.h"
#endif

// Preferences.
#include "QvisPreferencesDialog.h"
#include "QvisCapturePreferencesDialog.h"
#include "QvisMoviePreferencesDialog.h"

// Properties
#include "QvisVolumePropertiesDialog.h"
#include "QvisMeshPropertiesDialog.h"
#include "QvisViewPropertiesDialog.h"
#include "QvisOutputImagePropertiesDialog.h"

// Tools
#include "QvisCalculationDialog.h"
#include "QvisColocalizationDialog.h"
#include "QvisComponentAnalyzerDialog.h"
#include "QvisClipDialog.h"
#include "QvisConvertDialog.h"
#include "QvisExportDialog.h"
#include "QvisMeasurementDialog.h"
#include "QvisNoiseReductionDialog.h"
#include "QvisOpenCLKernelEditorDialog.h"
#include "QvisPaintBrushDialog.h"
#include "QvisTrackingDialog.h"
#include "QvisVolumeSizeDialog.h"

// Mesh
#include "QvisMeshTransformDialog.h"

#include "CapturePreferencesAttributes.h"
#include "MoviePreferencesAttributes.h"

#include <iostream>

QvisMainWindow::QvisMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QvisMainWindow),

    // Main windows
    mMainManager         (new QvisMainManager         (this)),
    mViewWindow          (new QvisViewWindow          (this)),
    mMessageDialog       (new QvisMessageDialog       (this)),

    // Preferences dialog
    mPreferencesDialog          (new QvisPreferencesDialog(this)),
    mCapturePreferencesDialog   (new QvisCapturePreferencesDialog(this)),
    mMoviePreferencesDialog     (new QvisMoviePreferencesDialog(this)),

    // Property dialogs
    mVolumePropertiesDialog     (new QvisVolumePropertiesDialog(this)),
    mMeshPropertiesDialog       (new QvisMeshPropertiesDialog(this)),
    mViewPropertiesDialog       (new QvisViewPropertiesDialog(this)),
    mOutputImagePropertiesDialog(new QvisOutputImagePropertiesDialog(this)),

    // Tool dialogs
    mCalculationDialog       (new QvisCalculationDialog(this)),
    mClipDialog              (new QvisClipDialog(this)),
    mColocalizationDialog    (new QvisColocalizationDialog(this)),
    mComponentAnalyzerDialog (new QvisComponentAnalyzerDialog(this)),
    mConvertDialog           (new QvisConvertDialog(this)),
    mExportDialog            (new QvisExportDialog(this)),
    mMeasurementDialog       (new QvisMeasurementDialog(this)),
    mNoiseReductionDialog    (new QvisNoiseReductionDialog(this)),
    mOpenCLKernelEditorDialog(new QvisOpenCLKernelEditorDialog(this)),
    mPaintBrushDialog        (new QvisPaintBrushDialog(this)),
    mTrackingDialog          (new QvisTrackingDialog(this)),
    mVolumeSizeDialog        (new QvisVolumeSizeDialog(this)),

    // Mesh Dialog
    mMeshTransformDialog        (new QvisMeshTransformDialog(this)),

    mCapturePreferencesAttributes (new CapturePreferencesAttributes()),
    mMoviePreferencesAttributes   (new MoviePreferencesAttributes())

{
    ui->setupUi(this);

    mCapturePreferencesDialog->setAttributes(mCapturePreferencesAttributes);
    mMoviePreferencesDialog->setAttributes(mMoviePreferencesAttributes);

#ifdef HAVE_ADS
    // Set up the dock manager. Config flags must be set BEFORE the CDockManager is created otherwise it will crash.
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    // Create the dock manager AFTER the UI is created because
    // the dock manager registers itself as the central widget.
    mDockManager = new ads::CDockManager(this);
#else
    // Setup the main window with a splitter.
    // Splitter between the data manager and the view.
    mMainSplitter = new QSplitter(Qt::Horizontal);

    // Data manager on the left.
    mMainSplitter->addWidget(mMainManager);

    // View window on the right
    mMainSplitter->addWidget(mViewWindow);

    mMainSplitter->setStretchFactor(0, 1);
    mMainSplitter->setStretchFactor(1, 5);

    int width = this->width();
    mMainSplitter->setSizes({width*1/5,width*4/5});  // ~1/5

    setCentralWidget(mMainSplitter);
#endif

    // Add to the menu bar the about and preference entries
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAction = helpMenu->addAction(tr("&About FluoRender"), this, &QvisMainWindow::ActionAbout);
    aboutAction->setStatusTip(tr("Show FluoRender's About box"));

    QAction *preferencesAction = helpMenu->addAction(QIcon(":Src/FluoRender/UI/icons/MainWindow/Preferences.png"), tr("&Preferences"), this, &QvisMainWindow::ActionPreferences);
    preferencesAction->setStatusTip(tr("Show FluoRender's Preferences dialog"));

    // These main menu signals go to the Source Manager - can not be set in the ui creator
    QObject::connect(ui->actionOpenProject,   SIGNAL(triggered()), mMainManager->getSourceManager(), SLOT(ProjectOpenClicked()));
    QObject::connect(ui->actionSaveProject,   SIGNAL(triggered()), mMainManager->getSourceManager(), SLOT(ProjectSaveClicked()));
    QObject::connect(ui->actionSaveProjectAs, SIGNAL(triggered()), mMainManager->getSourceManager(), SLOT(ProjectSaveAsClicked()));
    QObject::connect(ui->actionOpenVolume,    SIGNAL(triggered()), mMainManager->getSourceManager(), SLOT(VolumeOpenClicked()));
    QObject::connect(ui->actionOpenMesh,      SIGNAL(triggered()), mMainManager->getSourceManager(), SLOT(MeshOpenClicked()));

    // These main menu signals come from the Source Manager - can not be set in the ui creator
    QObject::connect(mMainManager->getSourceManager(), SIGNAL(projectDirectoryChanged(QString)), this, SLOT(ProjectDirectoryChanged(QString)));

    // These main menu signals come from the Workspace Manager - can not be set in the ui creator
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(updateViewWindow(QString)),        this, SLOT(UpdateViewWindow(QString)));
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(actionViewWindowAdd()),            this, SLOT(ActionViewWindowNew()));
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(actionViewWindowDelete(QString)),  this, SLOT(ActionViewWindowDelete(QString)));
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(actionVewWindowToggleView()),      this, SLOT(ActionViewWindowToggleView()));

    // Misc connections - should be handled through the preferences update.
    QObject::connect(mPreferencesDialog, SIGNAL(animationSliderModeChanged(int)), mMainManager->getAnimationController(), SLOT(AnimationSliderModeChanged(int)));
    QObject::connect(mPreferencesDialog, SIGNAL(animationPauseValueChanged(int)), mMainManager->getAnimationController(), SLOT(AnimationPauseValueChanged(int)));

    // Connect the managers to the message dialog.
    ConnectToMessageDialog(mMainManager->getSourceManager());
    ConnectToMessageDialog(mMainManager->getDatasetManager());
    ConnectToMessageDialog(mMainManager->getWorkspaceManager());

    ui->menuView->addSeparator();

#ifdef HAVE_ADS
   // Add the data manager and view window to the dock manager.
    // Data manager on the left.
    QString name = mMainManager->windowTitle();
    ads::CDockWidget* dataManagerDockWidget = new ads::CDockWidget(name);
    dataManagerDockWidget->setObjectName(name.simplified().remove(" "));
    // Create a dock widget and set the mMainManager as the dock widget content.
    dataManagerDockWidget->setWidget(mMainManager);
    // Add the toggleViewAction of the dock widget to the menu to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->addAction(dataManagerDockWidget->toggleViewAction());
    // Add the data widget to the left dock area
    ads::CDockAreaWidget* dockArea =
        mDockManager->setCentralWidget(dataManagerDockWidget);
//      mDockManager->addDockWidget(ads::LeftDockWidgetArea, dataManagerDockWidget);

    // View on the right.
    name = mViewWindow->windowTitle();
    ads::CDockWidget* viewWindowDockWidget = new ads::CDockWidget(name);
    viewWindowDockWidget->setObjectName(name.simplified().remove(" "));

    // Create a dock widget and set the mViewWindow as the dock widget content.
    viewWindowDockWidget->setWidget(mViewWindow);
    // Add the toggleViewAction of the dock widget to the menu to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->addAction(viewWindowDockWidget->toggleViewAction());

    // Add the view widget to the right dock area
//    ads::CDockAreaWidget* dockArea =
        mDockManager->addDockWidget(ads::RightDockWidgetArea, viewWindowDockWidget, dockArea);
//      mDockManager->setCentralWidget(viewWindowDockWidget);

     dockArea->setAllowedAreas(ads::DockWidgetArea::OuterDockAreas);

    // Set the splitter sizes so the data manager/view window has a reasonable split.
    ads::CDockSplitter* splitter = ads::internal::findParent<ads::CDockSplitter*>(dockArea);

    int width = this->width();
    // When using a central widget the splitter sizes are funky due to a bug.
    // It works as one would expect if there is no central widget.
    splitter->setSizes({-width*1/2,width*3/2});  // ~1/3
    //splitter->setSizes({-width*4/5,width*9/5});  // ~1/5

    // Add the message dialog
    name = mMessageDialog->windowTitle();
    ads::CDockWidget* messageDialogDockWidget = new ads::CDockWidget(name);
    messageDialogDockWidget->setObjectName(name.simplified().remove(" "));

    // Set the window size so the scrollbars are not present initially.
    int w = messageDialogDockWidget->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QRect geom = mMessageDialog->geometry(); // Get before setting as a widget.
    messageDialogDockWidget->setGeometry(geom.x(), geom.y(), geom.width()+w, geom.height()+w);

    // Set the widget as the dock widget content
    messageDialogDockWidget->setWidget(mMessageDialog);

    messageDialogDockWidget->toggleViewAction()->setIcon(mMessageDialog->windowIcon());

    mDockManager->addDockWidgetFloating(messageDialogDockWidget);

    // The message dialog dock widget will be floating but make it hidden initially.
    messageDialogDockWidget->toggleView(false);

    // Add the toggleViewAction of the dock widget to the menu to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->addAction(messageDialogDockWidget->toggleViewAction());

    // When using the ADS use the dockWidget's toggleViewAction instead
    // of the dialog's internal toggleViewAction.
    mMessageDialog->setToggleViewAction(messageDialogDockWidget->toggleViewAction());

    // Create a label and combobox for the dock perspectives.
    mDockLayoutToolbar = new QToolBar("Dock layout toolbar", this);
    mDockLayoutToolbar->setObjectName("DockLayoutToolbar");
    mDockLayoutToolbar->isMovable();
    // Insert the mDockLayoutToolbar in front of the mPropertiesToolbar.
    this->addToolBar(mDockLayoutToolbar);
    ui->menuView->addSeparator();
    ui->menuView->addAction(mDockLayoutToolbar->toggleViewAction());

    mDockLayoutToolbar->addWidget( new QLabel("Dock layout:", this) );

    QWidgetAction *perspectiveListAction = new QWidgetAction(this);
    mPerspectiveComboBox = new QComboBox(this);
    mPerspectiveComboBox->setObjectName("PerspectiveComboBox");
    mPerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mPerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mPerspectiveComboBox->setEditable(true);
    mPerspectiveComboBox->clearFocus();
    perspectiveListAction->setDefaultWidget(mPerspectiveComboBox);
    mDockLayoutToolbar->addAction(perspectiveListAction);
    QObject::connect(mPerspectiveComboBox, SIGNAL(textActivated(QString)),
        mDockManager, SLOT(openPerspective(QString)));

    // Use a functor so to be able to use ActionCreatePerspective for both the combobox
    // and the menu action. Note the menu action, set in the ui does NOT pass an argument.
    // As such the ActionCreatePerspective has a default value of true.
    QObject::connect(mPerspectiveComboBox->lineEdit(), &QLineEdit::returnPressed, this, [this]{ ActionCreatePerspective(false); });

    // The perspective delete and save menu triggers go to their own slots.
    QObject::connect(ui->menuDeletePerspective, SIGNAL(triggered(QAction*)), this, SLOT(ActionDeletePerspective(QAction*)));
    QObject::connect(ui->menuSavedPerspectives, SIGNAL(triggered(QAction*)), this, SLOT(ActionSetPerspective(QAction*)));

    QObject::connect(mDockManager, SIGNAL(focusedDockWidgetChanged(ads::CDockWidget*, ads::CDockWidget*)), this, SLOT(FocusedDockWidgetChanged(ads::CDockWidget*, ads::CDockWidget*)));

#else
    // Add the toggleViewAction of the dock widget to the menu to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->addAction(mMessageDialog->toggleViewAction());
    ui->menuView->addSeparator();

    // No ADS so remove the perspectives menu.
    for( QAction *action : ui->menuFiles->actions())
    {
        if(action->text() == "Perspectives")
            ui->menuFiles->removeAction(action);
    }

    QObject::connect(mViewWindow, SIGNAL(activeViewWindowChanged(QString,QEvent::Type)), this, SLOT(ActiveViewWindowChanged(QString,QEvent::Type)));
#endif

    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(activeViewWindowChanged(QString)),               this, SLOT(ActiveViewWindowChanged(QString)));
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(activeViewWindowVolumeChanged(QString,QString)), this, SLOT(ActiveViewWindowVolumeChanged(QString,QString)));
    QObject::connect(mMainManager->getWorkspaceManager(), SIGNAL(activeViewWindowMeshChanged(QString,QString)),   this, SLOT(ActiveViewWindowMeshChanged(QString,QString)));

    // The properties toolbar allows the user bring up properties dialogs so recieve those signals.
    mPropertiesToolbar = new QvisDragDropToolBar("Properties toolbar", this);
    mPropertiesToolbar->setObjectName("PropertiesToolbar");
    mPropertiesToolbar->setPixmapSize(20);
    mPropertiesToolbar->setMovable(true);
    mPropertiesToolbar->setHidden(true);
    mPropertiesToolbar->setSortSourceAction(true);
    mPropertiesToolbar->setGarbageCan(true);
    mPropertiesToolbar->setToggleCheckableActions(false);

    ui->menuView->addAction(mPropertiesToolbar->toggleViewAction());
    this->addToolBar(mPropertiesToolbar);

    // The tools toolbar allows the user bring up tool dialogs so recieve those signals.
    mToolsToolbar = new QvisDragDropToolBar("Tools toolbar", this);
    mToolsToolbar->setObjectName("ToolsToolbar");
    mPropertiesToolbar->setPixmapSize(20);
    mToolsToolbar->setMovable(true);
    mToolsToolbar->setHidden(true);
    mToolsToolbar->setGarbageCan(false);
    mToolsToolbar->setSortSourceAction(true);
    //mToolsToolbar->setToggleCheckableActions(false);

    ui->menuView->addAction(mToolsToolbar->toggleViewAction());
    this->addToolBar(mToolsToolbar);

    // Property dialogs as widgets
    this->AddDialog(mVolumePropertiesDialog,      ui->menuProperties, mPropertiesToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mMeshPropertiesDialog,        ui->menuProperties, mPropertiesToolbar, &mMainManager->getWorkspaceManager()->getMeshActionList());
    ui->menuProperties->addSeparator();
    this->AddDialog(mViewPropertiesDialog,        ui->menuProperties, mPropertiesToolbar, &mMainManager->getWorkspaceManager()->getViewWindowActionList());
    this->AddDialog(mOutputImagePropertiesDialog, ui->menuProperties, mPropertiesToolbar, &mMainManager->getWorkspaceManager()->getViewWindowActionList());

    // Tool dialogs as widgets
    this->AddDialog(mCalculationDialog,        ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mClipDialog,               ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mColocalizationDialog,     ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mComponentAnalyzerDialog,  ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mConvertDialog,            ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mExportDialog,             ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mMeasurementDialog,        ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mNoiseReductionDialog,     ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mOpenCLKernelEditorDialog, ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mPaintBrushDialog,         ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mTrackingDialog,           ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    this->AddDialog(mVolumeSizeDialog,         ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getVolumeActionList());
    ui->menuTools->addSeparator();
    this->AddDialog(mMeshTransformDialog,      ui->menuTools, mToolsToolbar, &mMainManager->getWorkspaceManager()->getMeshActionList());

    ui->menuView->addSeparator();

    mViewWindowMap[mViewWindow->windowTitle()] = mViewWindow;

    ConnectToMessageDialog(mViewWindow);
    // Actions for the view window context menu.
    mViewWindow->setViewWindowActionList(mMainManager->getWorkspaceManager()->getViewWindowActionList());
    // Update attributes.
    mViewWindow->setCapturePreferencesAttributes(mCapturePreferencesAttributes);

    mExportDialog->setMoviePreferencesDialog(mMoviePreferencesDialog);
    mExportDialog->setMoviePreferencesAttributes(mMoviePreferencesAttributes);

    // Connenct the current view window to the main menu.
    UpdateViewWindowSignals(true);

    mMainManager->getWorkspaceManager()->WorkspaceViewWindowAddClicked(mViewWindow->windowTitle());

    // Needed for keeping track of the active window.
//    QObject::connect(QApplication::instance(), SIGNAL(focusWindowChanged(QWindow*)),
//            this, SLOT(onFocusWindowChanged(QWindow*)));

    // Restore the previous dock perspectives.
    this->RestorePerspectives();

    // Restore the preivious layout state.
    ActionRestoreLayoutState();
}

QvisMainWindow::~QvisMainWindow()
{
    delete ui;
}

void QvisMainWindow::loadFile(QString filename)
{
    if(filename.endsWith(".vrp"))
        mMainManager->getSourceManager()->ProjectOpenClicked(filename);
    else if(filename.endsWith(".obj"))
        mMainManager->getSourceManager()->MeshOpenClicked(filename);
    else
        mMainManager->getSourceManager()->VolumeOpenClicked(filename);
}

void QvisMainWindow::closeEvent(QCloseEvent* event)
{
#ifdef HAVE_ADS
    // Delete dock manager here to delete all floating widgets. This ensures
    // that all top level windows of the dock manager are properly closed.
    mDockManager->deleteLater();

    // When closing ignore focus changes as windows have been deleted.
    QObject::disconnect(mDockManager, SIGNAL(focusedDockWidgetChanged(ads::CDockWidget*, ads::CDockWidget*)), this, SLOT(FocusedDockWidgetChanged(ads::CDockWidget*, ads::CDockWidget*)));
#endif

    QMainWindow::closeEvent(event);
}

// AddDialog
void QvisMainWindow::AddDialog(QvisDialogBase *dialog, QMenu *menu, QvisDragDropToolBar *toolbar, QList<QAction *> *actionList)
{
    QString  name = dialog->windowTitle();

    ConnectToMessageDialog(dialog);

#ifdef HAVE_ADS
    // Create a dock widget - the dock manager will delete it and the widget.
    ads::CDockWidget * dockWidget = new ads::CDockWidget(name, this);
    dockWidget->setObjectName(name.simplified().remove(" "));

    // Set the window size so the scrollbars are not present initially.
    int w = dockWidget->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QRect geom = dialog->geometry(); // Get before setting as a widget.
    dockWidget->setGeometry(geom.x(), geom.y(), geom.width()+w, geom.height()+w);

    // Set the widget as the dock widget content
    dockWidget->setWidget(dialog);

    dockWidget->toggleViewAction()->setIcon(dialog->windowIcon());

    // Add the toggleViewAction of the dock widget to the menu so to give
    // the user the ability to show the dock widget if it has been closed.
    menu->addAction(dockWidget->toggleViewAction());

    // Add the dialog to the toolbar so to give the user the
    // ability to show/hide the dialog in the toolbar.
    toolbar->addDragAction(dockWidget->toggleViewAction());

    // Add the toggleViewAction of the dock widget to the actionList in the
    // data manager which is used in the context menus.
    if(actionList)
        actionList->push_back(dockWidget->toggleViewAction());

    // Add the dock widget as a floating widget.
    mDockManager->addDockWidgetFloating(dockWidget);

    // The dock widget will be floating but make it hidden initially.
    dockWidget->toggleView(false);

    // When using the ADS use the dockWidget's toggleViewAction instead
    // of the dialog's internal toggleViewAction.
    dialog->setToggleViewAction(dockWidget->toggleViewAction());
#else
    dialog->toggleViewAction()->setIcon(dialog->windowIcon());

    // Add the toggleViewAction of the dialog to the menu so to give
    // the user the ability to show/hide the dialog.
    menu->addAction(dialog->toggleViewAction());

    // Add the dialog to the toolbar so to give the user the
    // ability to show/hide the dialog in the toolbar.
    toolbar->addDragAction(dialog->toggleViewAction());

    // Add the toggleViewAction of the dock widget to the actionList in the
    // data manager which is used in the context menus.
    if(actionList)
        actionList->push_back(dialog->toggleViewAction());
#endif

    if(dialog->hasDragDropToolBar())
    {
        this->addToolBar(dialog->dragDropToolBar());
        ui->menuView->addAction(dialog->dragDropToolBar()->toggleViewAction());
    }

    mDialogWidgets.push_back(dialog);
}

// Application

void QvisMainWindow::ActionAbout()
{
    QString ABOUT(FLUORENDER_TITLE " " FLUORENDER_VERSION_STRING
                  "\n\n"
                  "Copyright © " FLUORENDER_VERSION_COPYRIGHT
            );

    QMessageBox::about(this, "About FluoRender", ABOUT );
}

void QvisMainWindow::ActionPreferences()
{
    if (mPreferencesDialog->exec() == QDialog::Accepted)
    {
     // Update attributes.
    }
}

void QvisMainWindow::ActionCapturePreferences()
{
    mCapturePreferencesDialog->exec();
}

void QvisMainWindow::ActionMoviePreferences()
{
    mMoviePreferencesDialog->exec();
}

// View
void QvisMainWindow::ActionViewWindowNew(QvisViewWindow *viewWindow)
{
    // Disconnect the current view window from the main menu
    UpdateViewWindowSignals(false);

    // If the viewWindow is not null then an internal call as part of the state restoration.
    if(viewWindow != nullptr)
        mViewWindow = viewWindow;
    // Otherwise a UI call to create a new view window.
    else
        mViewWindow = new QvisViewWindow(this);

    QString name = mViewWindow->windowTitle();

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << viewWindow << "  " << mViewWindow->windowTitle().toStdString() << std::endl;

    if(name.isEmpty())
        return;

    mViewWindowMap[name] = mViewWindow;

    ConnectToMessageDialog(mViewWindow);
    // Actions for the view window context menu.
    mViewWindow->setViewWindowActionList(mMainManager->getWorkspaceManager()->getViewWindowActionList());
    // Update attributes.
    mViewWindow->setCapturePreferencesAttributes(mCapturePreferencesAttributes);

    // Connenct the current view window to the main menu.
    UpdateViewWindowSignals(true);

    mMainManager->getWorkspaceManager()->WorkspaceViewWindowAddClicked(name);

    // Find the last view window action so to insert a new toggle view action
    // for the new window.
    auto actions = ui->menuView->actions();
    int i = 0;

    QAction *viewAction = nullptr;
    for(auto action : actions)
    {
        // The insertAction inserts the new action before the action. So
        // save the action after the view window action.
        if(action->text().contains("View Window"))
            viewAction = actions[i+1];

        ++i;
    }

#ifdef HAVE_ADS
    // Create a dock widget - the dock manager will delete it and the widget.
    ads::CDockWidget * dockWidget = new ads::CDockWidget(name, this);
    dockWidget->setObjectName(name.simplified().remove(" "));

    // Set the window size so the scrollbars are not present initially.
    int w = dockWidget->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QRect geom = mViewWindow->geometry(); // Get before setting as a widget.
    dockWidget->setGeometry(geom.x(), geom.y(), geom.width()+w, geom.height()+w);

    // Set the widget as the dock widget content
    dockWidget->setWidget(mViewWindow);

    dockWidget->toggleViewAction()->setIcon(mViewWindow->windowIcon());

    // Add the toggleViewAction of the dock widget to the menu so to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->insertAction(viewAction, dockWidget->toggleViewAction());

    // Add the dock widget as a floating widget.
    mDockManager->addDockWidgetFloating(dockWidget);

    // The dock widget will be floating and make it visible.
    dockWidget->toggleView(true);
#else
    // Create a view window dialog
    QvisViewWindowDialog * dialog = new QvisViewWindowDialog(this);

    dialog->setWidget(mViewWindow);

    // Add the toggleViewAction of the dock widget to the menu so to give
    // the user the ability to show the dock widget if it has been closed.
    ui->menuView->insertAction(viewAction, dialog->toggleViewAction());

    dialog->toggleView(true);

    QObject::connect(mViewWindow, SIGNAL(activeViewWindowChanged(QString,QEvent::Type)), this, SLOT(ActiveViewWindowChanged(QString,QEvent::Type)));
#endif

    UpdateViewWindowList();
}


void QvisMainWindow::ActionViewWindowDelete(QString name)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << name.toStdString() << "'  " << std::endl;

    if(mViewWindowMap.find(name) != mViewWindowMap.end())
        mViewWindow = mViewWindowMap[name];
    else
        return;

    // Disconnenct the current view window from the main menu.
    UpdateViewWindowSignals(false);

#ifdef HAVE_ADS
    // Delete the dock widget.
    ads::CDockWidget* dockWidget = mDockManager->findDockWidget(name.simplified().remove(" "));

    if(dockWidget)
        mDockManager->removeDockWidget(dockWidget);
#else
    // If there is a dialog then a floating window so delete the
    // dialog and the view window.
    QvisViewWindowDialog * dialog = mViewWindow->getViewWindowDialog();

    if(dialog)
    {
        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << dialog->windowTitle().toStdString() << "  " << std::endl;

        dialog->deleteLater();
    }
#endif

    // Remove the name from the view window map.
    mViewWindowMap.erase(name);

    // Get the first view window in the map and make it the current view window.
    if(mViewWindowMap.empty())
        mViewWindow = nullptr; // This should not happen as there should always be one view window.
    else
        mViewWindow = mViewWindowMap.begin()->second;

    for (const auto& [key, _] : mViewWindowMap)
        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << key.toStdString() << std::endl;

    // Connenct the current view window to the main menu.
    UpdateViewWindowSignals(true);

#ifndef HAVE_ADS
    // If the to be deleted view window does not have a dialog then it was the base window in the spliter.
    // As such, remove it from the splitter and replace it with the cureent view window.
    if(!dialog)
    {
        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mViewWindow->windowTitle().toStdString() << "  " << std::endl;

        // Get the current view window dialog.
        dialog = mViewWindow->getViewWindowDialog();

        if(dialog)
        {
            std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << dialog->windowTitle().toStdString() << "  " << std::endl;
            // Remove the current view window from it's floating dialog as it will belong to the splitter.
            dialog->removeWidget(mViewWindow);

            // Replace the view window in the splitter with the current view window.
            QWidget *widget = mMainSplitter->replaceWidget(1, mViewWindow);
            mViewWindow->setParent(mMainSplitter);

            // Delete the old view window and the no longer needed floating dialog.
            widget->deleteLater();
            dialog->deleteLater();
        }
    }
#endif

    // Now remove the menu action;
    auto actions = ui->menuView->actions();

    for(auto action : actions)
    {
        if(action->text() == name)
        {
            ui->menuView->removeAction(action);
            break;
        }
    }

    // Remove the view window from the data manager.
    mMainManager->getWorkspaceManager()->WorkspaceSelectionDeleteClicked(name);

    UpdateViewWindowList();
}

void QvisMainWindow::ActionViewWindowToggleView()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mViewWindow->windowTitle().toStdString() << "  " << std::endl;

    if(mViewWindow == nullptr)
        return;

#ifdef HAVE_ADS
    ads::CDockWidget * dockWidget = mDockManager->findDockWidget(mViewWindow->windowTitle().simplified().remove(" "));

    if(dockWidget)
        dockWidget->toggleView(dockWidget->isClosed());
#else

    QvisViewWindowDialog * dialog = mViewWindow->getViewWindowDialog();

    if(dialog)
        dialog->toggleView(!dialog->isVisible());
#endif
}

// Help
void QvisMainWindow::ActionFluoRenderWebsite()
{
    QString VERSION_CONTACT("http://www.sci.utah.edu/software/fluorender.html");

    QDesktopServices::openUrl(QUrl(VERSION_CONTACT));
}

void QvisMainWindow::ActionCheckForUpdates()
{
    QString VERSION_UPDATES("http://www.sci.utah.edu/releases/fluorender_v"
                   FLUORENDER_VERSION_STRING
                   "/");

    QDesktopServices::openUrl(QUrl(VERSION_UPDATES));
}

void QvisMainWindow::ActionDownloadManual()
{
    QString HELP_MANUAL("https://github.com/SCIInstitute/fluorender/releases/download/v"\
                        FLUORENDER_VERSION_STRING
                        "/FluoRender"
                        FLUORENDER_VERSION_STRING
                        "_Manual.pdf");

    QDesktopServices::openUrl(QUrl(HELP_MANUAL));
}

void QvisMainWindow::ActionDownloadTutorial()
{
    QString HELP_TUTORIAL("https://github.com/SCIInstitute/fluorender/releases/download/v"\
                          FLUORENDER_VERSION_STRING
                          "/FluoRender"
                          FLUORENDER_VERSION_STRING
                          "_Tutorials.pdf");

    QDesktopServices::openUrl(QUrl(HELP_TUTORIAL));
}

void QvisMainWindow::ProjectDirectoryChanged(QString directory)
{
    mCapturePreferencesDialog->setCurrentProjectDirectory(directory);
    mMoviePreferencesDialog->setCurrentProjectDirectory(directory);
}

// View Window Management
#ifdef HAVE_ADS
void QvisMainWindow::FocusedDockWidgetChanged(ads::CDockWidget* previous, ads::CDockWidget* current)
{
    Q_UNUSED(previous);

    QString name = current->windowTitle();

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << name.toStdString() << std::endl;

    if(name.contains("View Window "))
    {
        // Disconnenct the current view window from the main menu.
        UpdateViewWindowSignals(false);

        mViewWindow = mViewWindowMap[name];

        // Connenct the current view window to the main menu.
        UpdateViewWindowSignals(true);

        mMainManager->getWorkspaceManager()->WorkspaceViewWindowSelected(name);
    }
}
#endif
/*
void QvisMainWindow::ActiveViewWindowChanged(QString name, QEvent::Type type)
{
    if(closing)
        return;
    if(name.contains("View Window "))
    {
        // If activating a view window update the current window.
        // If deactivating a view window make sure it is the current window.
        if((type == QEvent::WindowActivate   && mViewWindow->windowTitle() != name) ||
           (type == QEvent::WindowDeactivate && mViewWindow->windowTitle() == name))
        {
            // Disconnenct the current view window from the main menu.
            UpdateViewWindowSignals(false);

            mViewWindow = mViewWindowMap[name];

            // Connenct the current view window to the main menu.
            UpdateViewWindowSignals(true);
        }

        ads::CDockWidget * dockWidget = mDockManager->findDockWidget(name.simplified().remove(" "));

        if(dockWidget)
            dockWidget->toggleView(true);
    }
}
*/

void QvisMainWindow::UpdateViewWindow(QString name)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << std::endl;

    mViewWindowMap[name]->update();
}

void QvisMainWindow::ActiveViewWindowChanged(QString name, QEvent::Type type)
{
    if(mViewWindow == nullptr)
        mViewWindow = mViewWindowMap[name];

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << mViewWindow->windowTitle().toStdString() << "  "  << name.toStdString()
              << (type == QEvent::WindowActivate ? "  WindowActivate" :"  WindowDeactivate")
              << std::endl;

    // If activating a view window update the current window.
    // If deactivating a view window make sure it is the current window.
    if((type == QEvent::WindowActivate   && mViewWindow->windowTitle() != name) ||
       (type == QEvent::WindowDeactivate && mViewWindow->windowTitle() == name))
    {
        // Disconnenct the current view window from the main menu.
        UpdateViewWindowSignals(false);

#ifndef HAVE_ADS
        if(type == QEvent::WindowDeactivate && mViewWindow->windowTitle() == name)
        {
            QWidget *widget = mMainSplitter->widget(1);

            if(widget)
                name = widget->windowTitle();
            else
                return;

            std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << name.toStdString() << std::endl;
        }
#endif

        mViewWindow = mViewWindowMap[name];

        // Connenct the current view window to the main menu.
        UpdateViewWindowSignals(true);

        mMainManager->getWorkspaceManager()->WorkspaceViewWindowSelected(name);

        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << name.toStdString() << std::endl;
    }

#ifdef HAVE_ADS
    ads::CDockWidget * dockWidget = mDockManager->findDockWidget(name.simplified().remove(" "));

    if(dockWidget)
        dockWidget->toggleView(true);
#else
    if(mViewWindow->getViewWindowDialog())
    {
        mViewWindow->getViewWindowDialog()->show();
        mViewWindow->getViewWindowDialog()->raise();
    }
#endif
}

void QvisMainWindow::ActiveViewWindowVolumeChanged(QString name, QString volume)
{
    mViewWindowMap[name]->setActiveVolume(volume);
}

void QvisMainWindow::ActiveViewWindowMeshChanged  (QString name, QString mesh)
{
    mViewWindowMap[name]->setActiveMesh(mesh);
}

void QvisMainWindow::UpdateViewWindowSignals(bool connect) const
{
    if(mViewWindow == nullptr)
        return;

    // Connect the current view window to the main menu, paint and measurement dialog
    if(connect)
    {
        QObject::connect(ui->actionLayoutModeSingle,     SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeSingle()));
        QObject::connect(ui->actionLayoutModeQuad,       SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeQuad()));
        QObject::connect(ui->actionLayoutModeHorizontal, SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeHorizontal()));
        QObject::connect(ui->actionLayoutModeVertical,   SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeVertical()));

        ui->actionLayoutModeSingle    ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutSingle);
        ui->actionLayoutModeQuad      ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutQuad);
        ui->actionLayoutModeHorizontal->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutHorizontal);
        ui->actionLayoutModeVertical  ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutVertical);

        QObject::connect(mPaintBrushDialog,  SIGNAL(currentCursorMode(QString,QString)), mViewWindow, SLOT(CurrentCursorMode(QString,QString)));
        QObject::connect(mMeasurementDialog, SIGNAL(currentCursorMode(QString,QString)), mViewWindow, SLOT(CurrentCursorMode(QString,QString)));

        QObject::connect(mViewWindow, SIGNAL(updateViewWindowMenus(QString,bool)), this, SLOT(UpdateViewWindowMenus(QString,bool)));
    }
    // Disconnect the current view window to the main menu, paint and measurement dialog
    else
    {
        QObject::disconnect(ui->actionLayoutModeSingle,     SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeSingle()));
        QObject::disconnect(ui->actionLayoutModeQuad,       SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeQuad()));
        QObject::disconnect(ui->actionLayoutModeHorizontal, SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeHorizontal()));
        QObject::disconnect(ui->actionLayoutModeVertical,   SIGNAL(triggered()), mViewWindow, SLOT(LayoutModeVertical()));

        ui->actionLayoutModeSingle    ->setChecked(false);
        ui->actionLayoutModeQuad      ->setChecked(false);
        ui->actionLayoutModeHorizontal->setChecked(false);
        ui->actionLayoutModeVertical  ->setChecked(false);

        QObject::disconnect(mPaintBrushDialog,  SIGNAL(currentCursorMode(QString,QString)), mViewWindow, SLOT(CurrentCursorMode(QString,QString)));
        QObject::disconnect(mMeasurementDialog, SIGNAL(currentCursorMode(QString,QString)), mViewWindow, SLOT(CurrentCursorMode(QString,QString)));

        QObject::disconnect(mViewWindow, SIGNAL(updateViewWindowMenus(QString,bool)), this, SLOT(UpdateViewWindowMenus(QString,bool)));
    }

    UpdateViewWindowMenus(mViewWindow->windowTitle(), connect);
}

void QvisMainWindow::UpdateViewWindowMenus(QString name, bool connect) const
{
    if(mViewWindow == nullptr || mViewWindow->windowTitle() != name)
        return;

    if(connect)
    {
        ui->actionLayoutModeSingle    ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutSingle);
        ui->actionLayoutModeQuad      ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutQuad);
        ui->actionLayoutModeHorizontal->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutHorizontal);
        ui->actionLayoutModeVertical  ->setChecked(mViewWindow->layoutMode() == QvisViewWindow::LayoutVertical);
    }
    else
    {
        ui->actionLayoutModeSingle    ->setChecked(false);
        ui->actionLayoutModeQuad      ->setChecked(false);
        ui->actionLayoutModeHorizontal->setChecked(false);
        ui->actionLayoutModeVertical  ->setChecked(false);
    }
}

void QvisMainWindow::UpdateViewWindowList()
{
    QStringList keys;

    for (const auto& [key, _] : mViewWindowMap)
    {
        keys.push_back(key);
    }

    mExportDialog->ViewWindowListChanged(keys);
}

void QvisMainWindow::ConnectToMessageDialog(QWidget *widget) const
{
    QObject::connect(widget, SIGNAL(postInformationalMessage(QString)), mMessageDialog, SLOT(postInformationalMessage(QString)));
    QObject::connect(widget, SIGNAL(postWarningMessage(QString)),       mMessageDialog, SLOT(postWarningMessage(QString)));
    QObject::connect(widget, SIGNAL(postErrorMessage(QString)),         mMessageDialog, SLOT(postErrorMessage(QString)));
    QObject::connect(widget, SIGNAL(clearMessage()),                    mMessageDialog, SLOT(clearMessage()));
}

// Layout - Toolbar and menu

// Save and restore the window layout.
void QvisMainWindow::ActionSaveLayoutState()
{
    QSettings Settings("SCI", "FluoRender");

    Settings.clear();
    Settings.beginGroup("QvisMainWindow");

    Settings.setValue("QvisMainWindow/Geometry",   this->saveGeometry());
    Settings.setValue("QvisMainWindow/State",      this->saveState());

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << mViewWindowMap.size() << std::endl;

    for(auto & [name, viewWindow] : mViewWindowMap)
        viewWindow->saveState(Settings);

#ifdef HAVE_ADS
    Settings.setValue("QvisMainWindow/DockingState",      mDockManager->saveState());
#endif
    Settings.setValue("QvisMainWindow/ToolbarProperties", mPropertiesToolbar->saveState());
    Settings.setValue("QvisMainWindow/ToolbarTools",      mToolsToolbar->saveState());

    mMainManager->saveState(Settings);

    for(const auto & dialog : mDialogWidgets)
    {
        if(dialog->hasDragDropToolBar())
            Settings.setValue("QvisMainWindow/" + dialog->windowTitle(), dialog->dragDropToolBar()->saveState());
    }

    Settings.endGroup();
}

void QvisMainWindow::ActionRestoreLayoutState()
{
    QSettings Settings("SCI", "FluoRender");
    Settings.beginGroup("QvisMainWindow");

    // Check for geometry, if null then no defaults have been saved. Set a default screen size
    // and return as there will not be any additional defaults.
    if (Settings.value("QvisMainWindow/Geometry").isNull())
    {
        QRect screenGeometry = QApplication::screens()[0]->geometry();
        this->setGeometry(screenGeometry.width()*1/10, screenGeometry.height()*1/10,
                          screenGeometry.width()*8/10, screenGeometry.height()*8/10);
        return;
    }

    this->restoreGeometry  (Settings.value("QvisMainWindow/Geometry"  ).toByteArray());
    this->restoreState     (Settings.value("QvisMainWindow/State"     ).toByteArray());

    // Get the setting keys for the view windows with a local state.
    QStringList keys = Settings.allKeys().filter("QvisViewWindow").filter("/LocalState");

    // Get the id of for the first view window.
    int id = keys[0].remove("QvisViewWindow").remove("/LocalState").toInt();

    // Index of the first view window to be created.
    int start;

    // If the initial view window is "View Window 0" then restore the state
    // as nothing else needs to be done.
    if(id == 0)
    {
        mViewWindow->restoreState(Settings);

        start = 1;
    }
    // If the initial view window is not "View Window 0" is not present then
    // when using the ADS it can be be removed. But for when not using the ADS then
    // the view window needs to updated.
    else
    {
        // Save the name, "View Window 0" of the initial view window so it can be
        // be used for deleting / renaming.
        QString name = mViewWindow->windowTitle();

#ifdef HAVE_ADS
        // Update the dock widget name - while this step works the dock manager uses
        // the initial name to set the location and geometry. Unfortunately it gets
        // confused and when restoring does not update the location and geometry.
        //ads::CDockWidget* dockWidget = mDockManager->findDockWidget(name.simplified().remove(" "));

        //if(dockWidget)
        //{
        //    dockWidget->setWindowTitle(mViewWindow->windowTitle());
        //    dockWidget->setObjectName(mViewWindow->windowTitle().simplified().remove(" "));
        //}

        // Instead just delete it.
        ActionViewWindowDelete(name);

        start = 0;
#else
        // Update the id as it is used for getting the state then restore state.
        mViewWindow->updateID(id);
        mViewWindow->restoreState(Settings);

        // When not using the dockng system the initial view window is part of the
        // splitter. If it is replaced by a new view before being fully rendered it does not
        // show up. As such, the ActionViewWindowDelete can not be used. It would probably
        // work if the window was floating. So manually change the view window name everywhere.

        // If the name has changed update the view window map with the new name.
        std::map<QString, QvisViewWindow *>::node_type nodeHandler = mViewWindowMap.extract(name);

        nodeHandler.key() = mViewWindow->windowTitle();
        mViewWindowMap.insert(std::move(nodeHandler));

        // Update the data manager with the new name too.
        mMainManager->WorkspaceViewWindowRename(name, mViewWindow->windowTitle());
        mMainManager->WorkspaceViewWindowSelected(mViewWindow->windowTitle());

        start = 1;
#endif
    }

    // Create and restore state for the rest of the views.
    for(int i=start; i<keys.size(); ++i)
    {
        int id = keys[i].remove("QvisViewWindow").remove("/LocalState").toInt();
        QvisViewWindow *viewWindow = new QvisViewWindow(id, this);
        viewWindow->restoreState(Settings);

        ActionViewWindowNew(viewWindow);
    }

#ifdef HAVE_ADS
    mDockManager->restoreState      (Settings.value("QvisMainWindow/DockingState").toByteArray());
#endif
    mPropertiesToolbar->restoreState(Settings.value("QvisMainWindow/ToolbarProperties").toByteArray());
    mToolsToolbar->restoreState     (Settings.value("QvisMainWindow/ToolbarTools").toByteArray());

    mMainManager->restoreState(Settings);

    for(auto dialog : mDialogWidgets)
    {
        if(dialog->hasDragDropToolBar())
            dialog->dragDropToolBar()->restoreState(Settings.value("QvisMainWindow/" + dialog->windowTitle()).toByteArray());
    }

    Settings.endGroup();

    UpdateViewWindowList();
}

// Perspective - Toolbar and menu
void QvisMainWindow::ActionCreatePerspective(bool needName)
{
#ifdef HAVE_ADS
    QString name;

    // NeedName is defaulted to true so when called by the menu action nothing is passed.
    // However when called by the combobox needName is set to false via a functor.
    if(needName)
    {
        // Get a new perspective name - if the name matches and old name then overwrite.
        name = QInputDialog::getText(this, "Save perspective", "Enter a unique perspective name:");
    }
    else
    {
        name = mPerspectiveComboBox->currentText();
    }

    if (!name.isEmpty())
    {
        mDockManager->addPerspective(name);

        this->PopulatePerspectiveMenus();

        QSignalBlocker Blocker(mPerspectiveComboBox);
        mPerspectiveComboBox->setCurrentText(name);
        mPerspectiveComboBox->clearFocus();
        this->SavePerspective();
    }
#endif
}

void QvisMainWindow::ActionDeletePerspective(QAction *action)
{
#ifdef HAVE_ADS
    // The action is defaulted to nullptr so when called by the
    // toolbar nothing is passed and the current combox text is used.
    // However when called by the menu the text from the action is used.

    // Delete the selected perspective.
    if(action == nullptr)
        mDockManager->removePerspectives(QList{mPerspectiveComboBox->currentText()});
    else
        mDockManager->removePerspectives(QList{action->text()});

    this->PopulatePerspectiveMenus();
#endif
}

void QvisMainWindow::ActionDeleteAllPerspectives()
{
#ifdef HAVE_ADS
    // Delete all of the save perspectives.
    mDockManager->removePerspectives(mDockManager->perspectiveNames());
    mPerspectiveComboBox->clear();
    ui->menuDeletePerspective->clear();
    ui->menuSavedPerspectives->clear();

    // Do a save perspective to clear out the settings.
    this->SavePerspective();
#endif
}

void QvisMainWindow::ActionSetPerspective(QAction *action)
{
#ifdef HAVE_ADS
    // Open the perspective via a menu action.
    mDockManager->openPerspective(action->text());

    QSignalBlocker Blocker(mPerspectiveComboBox);
    mPerspectiveComboBox->setCurrentText(action->text());
    mPerspectiveComboBox->clearFocus();
#endif
}

// Perspective - Helpers.
void QvisMainWindow::SavePerspective()
{
#ifdef HAVE_ADS
    QSettings Settings("Settings.ini", QSettings::IniFormat);
    mDockManager->savePerspectives(Settings);
#endif
}

void QvisMainWindow::RestorePerspectives()
{
#ifdef HAVE_ADS
    // Restore the setting from a previous session
    QSettings Settings("Settings.ini", QSettings::IniFormat);
    mDockManager->loadPerspectives(Settings);

    this->PopulatePerspectiveMenus();
#endif
}

void QvisMainWindow::PopulatePerspectiveMenus()
{
#ifdef HAVE_ADS
    QList<QString> pNames = mDockManager->perspectiveNames();

    // Repopulate the perspective combo box
    mPerspectiveComboBox->clear();
    mPerspectiveComboBox->addItems(pNames);

    // Repopulate the delete and saved perspective menus.
    ui->menuDeletePerspective->clear();
    ui->menuSavedPerspectives->clear();

    while (!pNames.isEmpty())
    {
        QString name = pNames.takeFirst();
        ui->menuDeletePerspective->addAction(name);
        ui->menuSavedPerspectives->addAction(name);
    }
#endif
}

void QvisMainWindow::onFocusWindowChanged(QWindow* window)
{
    if(window)
        std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "  << window->title().toStdString() << "  "
                  << std::endl;
}
