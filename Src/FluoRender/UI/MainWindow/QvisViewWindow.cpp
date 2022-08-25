#include "QvisViewWindow.h"
#include "ui_QvisViewWindow.h"

#include "QvisDragDropToolBar.h"
#include "QvisViewWindowDialog.h"

#include "CapturePreferencesAttributes.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QXmlStreamWriter>

#include <iostream>
#include <fstream>

int QvisViewWindow::mNumViewWindows = 0;

QvisViewWindow::QvisViewWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisViewWindow),
    mID(mNumViewWindows++)
{
    setup();
}

QvisViewWindow::QvisViewWindow(int id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisViewWindow),
    mID(id)
{
    setup();
}

void QvisViewWindow::setup()
{
    ui->setupUi(this);

    ui->ToolLayout->setContentsMargins(0, 0, 0, 0);
    ui->ToolLayout->insertWidget(0, dragDropToolBar());

    dragDropToolBar()->setPixmapSize(18);

    // Move the tool buttons to the drag and drop tool bar.
    QAction *action = new QAction(ui->CaptureButton->icon(), ui->CaptureButton->text(), this);
    action->setToolTip(ui->CaptureButton->toolTip());
    dragDropToolBar()->addDragAction(action);
    QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(CaptureButtonClicked()));
    ui->CaptureButton->setHidden(true);

    installEventFilter(this);

    updateID(mID);

    // Easier access to the four subwindows.
    mOpenGLWidget[0] = ui->OpenGLWidget_0;
    mOpenGLWidget[1] = ui->OpenGLWidget_1;
    mOpenGLWidget[2] = ui->OpenGLWidget_2;
    mOpenGLWidget[3] = ui->OpenGLWidget_3;

    mOpenGLWidget[1]->hide();
    mOpenGLWidget[2]->hide();
    mOpenGLWidget[3]->hide();

    // Initially only one view so hide the spliter option.
    LayoutModeSingle();

    // Having the view and splitter mode visible in the view window maybe annoying
    // as they are used very little so hide them.
    // They can not be destroyed as the combo box signals/slots are used by the context menu.
    if(mHideModeGroupBox)
        ui->ModeGroupBox->hide();

    setActiveVolume(tr(""));
    setActiveMesh(tr(""));
    CurrentCursorMode(tr(""), tr(""));
}

QvisViewWindow::~QvisViewWindow()
{
    delete ui;
}

QByteArray QvisViewWindow::saveState(int version) const
{
    QByteArray xmldata;
    QXmlStreamWriter s(&xmldata);

    // No spaces allowed in the element name.
    QString name = windowTitle();
    name.replace(" ", "_");

    // There may be multiple view windows so use the class name with the
    // window title as a unique identifier - note the underscore, no spaces.
    s.writeStartDocument();
        s.writeStartElement("QvisViewWindow_" + name);
        s.writeAttribute("Version", QString::number(mCurrentVersion));
        s.writeAttribute("UserVersion", QString::number(version));
        s.writeAttribute("ID", QString::number(mID));
        s.writeAttribute("LayoutMode", QString::number(mLayoutMode));
        s.writeAttribute("SplitterMode", QString::number(mSplitterMode));

        for(int i=0; i<mNumLayoutModes; ++i)
            s.writeAttribute(QString("ViewMode%1").arg(i), QString::number(mViewMode[i]));

        for(int i=0; i<mNumSubWindows; ++i)
            s.writeAttribute(QString("WindonwViewMode%1").arg(i), QString::number(mSubWindowViewMode[i]));

        s.writeEndElement();
    s.writeEndDocument();

    return xmldata;
}

bool QvisViewWindow::restoreState(const QByteArray &state, int version)
{
    if(state.isEmpty())
        return false;

    // Prevent multiple calls as long as state is not restore. This may
    // happen, if QApplication::processEvents() is called somewhere
    if (mRestoringState)
        return false;

    mRestoringState = true;

    QXmlStreamReader s(state);

    // No spaces allowed in the element name.
    QString name = windowTitle();
    name.replace(" ", "_");

    // Read the start element - note the underscore, no spaces.
    s.readNextStartElement();

    if (s.name() != (QLatin1String("QvisViewWindow_") + name))
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

    // Read the id - should already be set.
    int id = s.attributes().value("ID").toInt(&ok);
    if (!ok || id < 0 || id != mID)
        return false;

    updateID(id);

    // Read the mode for each four layouts.
    for(int i=0; i<mNumLayoutModes; ++i)
    {
        int tmp = s.attributes().value(QString("ViewMode%1").arg(i)).toInt(&ok);
        if (!ok || tmp < 0)
            return false;

        mViewMode[i] = ViewMode(tmp);
    }

    // Read the mode for each four windows.
    for(int i=0; i<mNumSubWindows; ++i)
    {
        int tmp = s.attributes().value(QString("WindonwViewMode%1").arg(i)).toInt(&ok);
        if (!ok || tmp < 0)
            return false;

        mSubWindowViewMode[i] = ViewMode(tmp);
    }

    // Read the SplitterMode.
    int tmp = s.attributes().value("SplitterMode").toInt(&ok);
    if (!ok || tmp < 0)
        return false;

    mSplitterMode = SplitterMode(tmp);

    // Reset the view which will reset the view and window modes.
    ui->SplitterModeComboBox->setCurrentIndex(mSplitterMode);

    // Read the LayoutMode.
    tmp = s.attributes().value("LayoutMode").toInt(&ok);
    if (!ok || tmp < 0)
        return false;

    mLayoutMode = QvisViewWindow::LayoutUnknown;

    switch(LayoutMode(tmp))
    {
    case QvisViewWindow::LayoutSingle:
        LayoutModeSingle();
        break;
    case QvisViewWindow::LayoutVertical:
        LayoutModeVertical();
        break;
    case QvisViewWindow::LayoutHorizontal:
        LayoutModeHorizontal();
        break;
    case QvisViewWindow::LayoutQuad:
        LayoutModeQuad();
        break;
    default:
        LayoutModeSingle();
        break;
    }

    mRestoringState = false;

    return true;
}

void QvisViewWindow::saveState(QSettings &Settings) const
{
    QString name = QString("QvisViewWindow%1/").arg(mID);

    if(mViewWindowDialog)
        Settings.setValue(name+"Geometry",   mViewWindowDialog->saveGeometry());

    Settings.setValue(name+"LocalState",     this              ->saveState());
    Settings.setValue(name+"MainSplitter",   ui->MainSplitter  ->saveState());
    Settings.setValue(name+"TopSplitter",    ui->TopSplitter   ->saveState());
    Settings.setValue(name+"BottomSplitter", ui->BottomSplitter->saveState());
}

void QvisViewWindow::restoreState(const QSettings &Settings)
{
    QString name = QString("QvisViewWindow%1/").arg(mID);

    if(mViewWindowDialog)
        mViewWindowDialog->restoreGeometry(Settings.value(name+"Geometry").toByteArray());

    this              ->restoreState(Settings.value(name+"LocalState")    .toByteArray());
    ui->MainSplitter  ->restoreState(Settings.value(name+"MainSplitter").  toByteArray());
    ui->TopSplitter   ->restoreState(Settings.value(name+"TopSplitter").   toByteArray());
    ui->BottomSplitter->restoreState(Settings.value(name+"BottomSplitter").toByteArray());
}

QvisViewWindowDialog * QvisViewWindow::getViewWindowDialog() const
{
    return mViewWindowDialog;
}

void QvisViewWindow::setViewWindowDialog(QvisViewWindowDialog *dialog)
{
    mViewWindowDialog = dialog;
}

bool QvisViewWindow::hasDragDropToolBar() const
{
    return mDragDropToolbar != nullptr;
}

QvisDragDropToolBar * QvisViewWindow::dragDropToolBar()
{
    if(mDragDropToolbar == nullptr)
    {
        // The toolbar allows the user to customize actions.
        mDragDropToolbar = new QvisDragDropToolBar(this->windowTitle(), this);
        mDragDropToolbar->setObjectName(this->windowTitle().simplified().remove(" ") + "Toolbar");
        mDragDropToolbar->isMovable();
        mDragDropToolbar->isHidden();
        mDragDropToolbar->setPixmapSize(8);
        QRect geom = mDragDropToolbar->geometry();
        geom.setHeight(geom.height()*2);
        mDragDropToolbar->setGeometry(geom);
    }

    return mDragDropToolbar;
}

int QvisViewWindow::activeSubWindowIndex() const
{
    return mActiveSubWindowIndex;
}


QvisViewWindow::LayoutMode QvisViewWindow::layoutMode() const
{
    return mLayoutMode;
}

void QvisViewWindow::setActiveVolume(QString name)
{
    ui->ActiveVolumeLabel->setHidden(name.isEmpty());
    ui->ActiveVolume->setHidden(name.isEmpty());
    ui->ActiveVolume->setText(name);
}

void QvisViewWindow::setActiveMesh(QString name)
{
    ui->ActiveMeshLabel->setHidden(name.isEmpty());
    ui->ActiveMesh->setHidden(name.isEmpty());
    ui->ActiveMesh->setText(name);
}

bool QvisViewWindow::eventFilter(QObject* watched, QEvent* event)
{
    //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "' " << event->type() << std::endl;

    // This method is for telling the data manager that a view window is truly active.
    // If the view window is part of the main window will become active when the main window
    // is active. However being active will trigger the workspace tree to be updated which is
    // not desirable. As such, only emit the signal iff there is a mouse down event.

    if(//event->type() == QEvent::WindowActivate ||
       event->type() == QEvent::MouseButtonPress)
    {
        //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "'  WindowActivate" << std::endl;
        emit activeViewWindowChanged(this->windowTitle(), QEvent::WindowActivate);
    }
    else if(event->type() == QEvent::WindowDeactivate)
    {
        //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "'  WindowDeactivate" << std::endl;
        //emit activeViewWindowChanged(this->windowTitle(), QEvent::WindowDeactivate);
    }

    return QWidget::eventFilter(watched, event);
}

// Views
void QvisViewWindow::LayoutModeSingle()
{
    if(mLayoutMode == QvisViewWindow::LayoutSingle)
        return;

    ExitFullScreenMode();

    ui->SplitterModeComboBox->setCurrentIndex(QvisViewWindow::Fixed);

    mLayoutMode = QvisViewWindow::LayoutSingle;

    mOpenGLWidget[1]->hide();
    mOpenGLWidget[2]->hide();
    mOpenGLWidget[3]->hide();

    ui->SplitterModeLabel->hide();
    ui->SplitterModeComboBox->hide();

    ViewModeUpdate();
}

void QvisViewWindow::LayoutModeHorizontal()
{
    ExitFullScreenMode();

    if(mLayoutMode == QvisViewWindow::LayoutHorizontal)
        return;

    mLayoutMode = QvisViewWindow::LayoutHorizontal;

    if(mSplitterMode == QvisViewWindow::Movable)
    {
        //int width = this->width();
        int height = this->height();

        const QSignalBlocker blockerMain  (ui->MainSplitter);
        const QSignalBlocker blockerTop   (ui->TopSplitter);
        const QSignalBlocker blockerBottom(ui->BottomSplitter);

        ui->MainSplitter  ->setSizes(QList{height/2, height/2});
        //ui->TopSplitter   ->setSizes(QList{width/2, width/2});
        //ui->BottomSplitter->setSizes(QList{width/2, width/2});
    }

    mOpenGLWidget[1]->hide();
    mOpenGLWidget[2]->show();
    mOpenGLWidget[3]->hide();

    ui->SplitterModeLabel->show();
    ui->SplitterModeComboBox->show();

    ViewModeUpdate();
}

void QvisViewWindow::LayoutModeVertical()
{
    ExitFullScreenMode();

    if(mLayoutMode == QvisViewWindow::LayoutVertical)
        return;

    mLayoutMode = QvisViewWindow::LayoutVertical;

    if(mSplitterMode == QvisViewWindow::Movable)
    {
        int width = this->width();
        //int height = this->height();

        const QSignalBlocker blockerMain  (ui->MainSplitter);
        const QSignalBlocker blockerTop   (ui->TopSplitter);
        const QSignalBlocker blockerBottom(ui->BottomSplitter);

        //ui->MainSplitter  ->setSizes(QList{height/2, height/2});
        ui->TopSplitter   ->setSizes(QList{width/2, width/2});
        ui->BottomSplitter->setSizes(QList{width/2, width/2});
    }

    mOpenGLWidget[1]->show();
    mOpenGLWidget[2]->hide();
    mOpenGLWidget[3]->hide();

    ui->SplitterModeLabel->show();
    ui->SplitterModeComboBox->show();

    ViewModeUpdate();
}

void QvisViewWindow::LayoutModeQuad()
{
    ExitFullScreenMode();

    if(mLayoutMode == QvisViewWindow::LayoutQuad)
        return;

    mLayoutMode = QvisViewWindow::LayoutQuad;

    if(mSplitterMode == QvisViewWindow::Movable)
    {
        int width  = this->width();
        int height = this->height();

        const QSignalBlocker blockerMain  (ui->MainSplitter);
        const QSignalBlocker blockerTop   (ui->TopSplitter);
        const QSignalBlocker blockerBottom(ui->BottomSplitter);

        ui->MainSplitter  ->setSizes(QList{height/2, height/2});
        ui->TopSplitter   ->setSizes(QList{width/2, width/2});
        ui->BottomSplitter->setSizes(QList{width/2, width/2});
    }

    mOpenGLWidget[1]->show();
    mOpenGLWidget[2]->show();
    mOpenGLWidget[3]->show();

    ui->SplitterModeLabel->show();
    ui->SplitterModeComboBox->show();

    ViewModeUpdate();
}

void QvisViewWindow::CurrentCursorMode(QString tool, QString mode)
{
    if(tool.isEmpty() || mode.isEmpty())
    {
        ui->ActiveToolModeLabel->hide();
        ui->ActiveTool->hide();
        ui->ActiveMode->hide();

        ui->ActiveTool->setText("");
        ui->ActiveMode->setText("");
    }
    else
    {
        ui->ActiveToolModeLabel->show();
        ui->ActiveTool->show();
        ui->ActiveMode->show();

        ui->ActiveTool->setText(tool);
        ui->ActiveMode->setText(mode);
    }
}

void QvisViewWindow::ViewModeUpdate()
{
    // Update the view sub window mode choices which are dependent on the current view layout.
    QSignalBlocker blocker(ui->ViewModeComboBox);

    while(ui->ViewModeComboBox->count())
        ui->ViewModeComboBox->removeItem(0);

    ui->ViewModeComboBox->addItems(mViewModes[mLayoutMode]);

    // Update the current item to the last recorded mode.
    if(mLayoutMode == QvisViewWindow::LayoutQuad)
        ui->ViewModeComboBox->setCurrentIndex(mViewMode[mLayoutMode] == QvisViewWindow::View_3D);
    else
        ui->ViewModeComboBox->setCurrentIndex(mViewMode[mLayoutMode]);

    // Alert that the mode had changed.
    ViewModeIndexChanged(ui->ViewModeComboBox->currentIndex());

    // Signal the main window that the menus have changed.
    emit updateViewWindowMenus(this->windowTitle(), true);
 }

void QvisViewWindow::ViewModeIndexChanged(int index)
{
    // Based on the view layout and the view mode set the view for each sub window.
    switch(mLayoutMode)
    {
    case QvisViewWindow::LayoutSingle:
        mSubWindowViewMode[0] = ViewMode(index);
        break;

    case QvisViewWindow::LayoutVertical:
    case QvisViewWindow::LayoutHorizontal:
        mSubWindowViewMode[0] = QvisViewWindow::View_3D;
        mSubWindowViewMode[1] = ViewMode(index);
        mSubWindowViewMode[2] = ViewMode(index);
        break;

    case QvisViewWindow::LayoutQuad:
    default:
        mSubWindowViewMode[0] = QvisViewWindow::View_3D;
        mSubWindowViewMode[1] = index ? QvisViewWindow::View_3D : QvisViewWindow::View_X;
        mSubWindowViewMode[2] = index ? QvisViewWindow::View_3D : QvisViewWindow::View_Y;
        mSubWindowViewMode[3] = index ? QvisViewWindow::View_3D : QvisViewWindow::View_Z;
        break;
    }

    // Save the view mode for the current layout.
    if(mLayoutMode == QvisViewWindow::LayoutQuad)
        mViewMode[mLayoutMode] = index ? QvisViewWindow::View_3D : QvisViewWindow::View_XYZ;
    else
        mViewMode[mLayoutMode] = ViewMode(index);
}

void QvisViewWindow::SplitterModeIndexChanged(int index)
{
    //    std::cerr << __FUNCTION__ << __LINE__ << std::endl;
    // Update the sub window layout to used either a fixed grid or movable splitters.
    if(mSplitterMode != SplitterMode(index))
    {
        mSplitterMode = SplitterMode(index);

        if(mSplitterMode == QvisViewWindow::Fixed)
        {
            // When items are removed from splitters and the splitter are hidden,
            // the state may not be preserved so store the state.
            mMainSplitterData   = ui->MainSplitter  ->saveState();
            mTopSplitterData    = ui->TopSplitter   ->saveState();
            mBottomSplitterData = ui->BottomSplitter->saveState();

            ui->MainSplitter->hide();

            // Adding the item to the layout removes it from the splitter.
            ui->OpenGLGridLayout->addWidget(mOpenGLWidget[0], 0, 0, 1, 1);
            ui->OpenGLGridLayout->addWidget(mOpenGLWidget[1], 0, 1, 1, 1);
            ui->OpenGLGridLayout->addWidget(mOpenGLWidget[2], 1, 0, 1, 1);
            ui->OpenGLGridLayout->addWidget(mOpenGLWidget[3], 1, 1, 1, 1);
        }
        else //if(mSplitterMode == QvisViewWindow::Movable)
        {
            ui->OpenGLGridLayout->removeWidget(mOpenGLWidget[0]);
            ui->OpenGLGridLayout->removeWidget(mOpenGLWidget[1]);
            ui->OpenGLGridLayout->removeWidget(mOpenGLWidget[2]);
            ui->OpenGLGridLayout->removeWidget(mOpenGLWidget[3]);

            ui->TopSplitter   ->addWidget(mOpenGLWidget[0]);
            ui->TopSplitter   ->addWidget(mOpenGLWidget[1]);
            ui->BottomSplitter->addWidget(mOpenGLWidget[2]);
            ui->BottomSplitter->addWidget(mOpenGLWidget[3]);

            // After adding the items restore the splitter state.
            ui->MainSplitter  ->restoreState(mMainSplitterData);
            ui->TopSplitter   ->restoreState(mTopSplitterData);
            ui->BottomSplitter->restoreState(mBottomSplitterData);

            ui->MainSplitter->show();
        }
    }
}

void QvisViewWindow::SplitterMoved(int pos, int index)
{
    Q_UNUSED(index);

    // The top and bottom splitters are locked so to move together like a single splitter.
    const QSignalBlocker blockerTop   (ui->TopSplitter);
    const QSignalBlocker blockerBottom(ui->BottomSplitter);

    QList topSize    = ui->TopSplitter   ->sizes();
    QList bottomSize = ui->BottomSplitter->sizes();

    if(pos == topSize[0])
        bottomSize = topSize;
    else if(pos == bottomSize[0])
        topSize = bottomSize;

    ui->TopSplitter   ->setSizes(topSize);
    ui->BottomSplitter->setSizes(bottomSize);
}

void QvisViewWindow::ContextMenuRequested(const QPoint &pos)
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mActiveSubWindowIndex << std::endl;

    if(mActiveSubWindowIndex < 0 || mNumSubWindows <= mActiveSubWindowIndex)
        return;

    QAction *action;
    QMenu menu(this);
    QMenu layoutModeMenu("View Layout", this);

    action = layoutModeMenu.addAction(tr("Single"),       this, &QvisViewWindow::LayoutModeSingle);
    action->setCheckable(true);
    action->setChecked(mLayoutMode == QvisViewWindow::LayoutSingle);
    action = layoutModeMenu.addAction(tr("Vertical"),   this, &QvisViewWindow::LayoutModeVertical);
    action->setCheckable(true);
    action->setChecked(mLayoutMode == QvisViewWindow::LayoutVertical);
    action = layoutModeMenu.addAction(tr("Horizontal"), this, &QvisViewWindow::LayoutModeHorizontal);
    action->setCheckable(true);
    action->setChecked(mLayoutMode == QvisViewWindow::LayoutHorizontal);
    action = layoutModeMenu.addAction(tr("Quad"),         this, &QvisViewWindow::LayoutModeQuad);
    action->setCheckable(true);
    action->setChecked(mLayoutMode == QvisViewWindow::LayoutQuad);

    menu.addMenu(&layoutModeMenu);

    // Menu for the view modes
    QMenu viewModeMenu("View Mode", this);
    int cc = 0;
    for(const auto & mode : mViewModes[mLayoutMode])
    {
        // Use a lambda function to set the view mode state.
        QAction *action = viewModeMenu.addAction(mode, this, [=] { ui->ViewModeComboBox->setCurrentText(mode); });
        action->setCheckable(true);
        if(mLayoutMode == QvisViewWindow::LayoutQuad)
            action->setChecked((mViewMode[mLayoutMode] == QvisViewWindow::View_3D) == cc);
        else
            action->setChecked(mViewMode[mLayoutMode] == ViewMode(cc));

        ++cc;
    }

    menu.addMenu(&viewModeMenu);

    // Optional menu for the splitter modes.
    QMenu splitterModeMenu("Splitters", this);
    if(mLayoutMode != QvisViewWindow::LayoutSingle)
    {
        int cc = 0;
        for(const auto & mode : mSplitterModes)
        {
            // Use a lambda function to set the splitter mode state.
            QAction *action = splitterModeMenu.addAction(mode, this, [=] { ui->SplitterModeComboBox->setCurrentText(mode); });
            action->setCheckable(true);
            action->setChecked(mSplitterMode == SplitterMode(cc));

            ++cc;
        }

        menu.addMenu(&splitterModeMenu);
    }

    // Add actions that toggle dialogs that are relevant to view windows
    if(mViewWindowActionList.size())
    {
        menu.addSeparator();

        for(auto *action : mViewWindowActionList)
            menu.addAction(action);
    }

    // Only one subwindow can be in full screen. The limitation is is only due to having one index (bookkeeping).
    if(mFullScreenSubWindowIndex == -1)
    {
        menu.addSeparator();
        menu.addAction(tr("Enter Full Screen"), this, SLOT(EnterFullScreenMode()));

        // The mActiveSubWindowIndex can change even when the context menu is present because the
        // mouse can leave the OpenGl windows thus issuing a leave event. So store the index so that
        // the full screen has the correct index.
        mContextSubWindowIndex = mActiveSubWindowIndex;
    }

    menu.exec(pos);
}

void QvisViewWindow::InitializeWindow(const QString name)
{
//    std::cerr << __FUNCTION__ << "  " << __LINE__ << std::endl;
    ActiveSubWindow(name);
}

void QvisViewWindow::PaintWindow(const QString name)
{
    //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << name.toStdString() << "  " << mActiveSubWindowIndex << std::endl;

    ActiveSubWindow(name);

    // Clear color and depth buffer
    switch(mActiveSubWindowIndex)
    {
    case -1:
        break;
    case 0:
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        break;
    case 1:
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        break;
    case 2:
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        break;
    case 3:
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        break;
    default:
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        break;
    }

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void QvisViewWindow::ResizeWindow(const QString name, const int w, const int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    //    std::cerr << __FUNCTION__ << "  " << __LINE__ << std::endl;
    ActiveSubWindow(name);
}

void QvisViewWindow::ActiveSubWindow(const QString name)
{
    if(name.isEmpty())
    {
        mActiveSubWindowIndex = -1;
    }
    else
    {
        for(int i=0; i<mNumSubWindows; ++i)
        {
            if(mOpenGLWidget[i]->objectName() == name)
            {
                if(mActiveSubWindowIndex != i)
                {
                    mActiveSubWindowIndex = i;

                    emit activeWindowIndexChanged(mActiveSubWindowIndex);

                    break;
                }
            }
        }
    }

//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << windowTitle().toStdString() << "  "
//              << name.toStdString() << "  " << mActiveSubWindowIndex << "  " << std::endl;
}

void QvisViewWindow::MousePress(const QPointF &point)
{
//    std::cerr << __FUNCTION__ << "  " << __LINE__  << "  " << mActiveSubWindowIndex << "  "
//              << point.x() << "  " << point.y() << "  "
//              << std::endl;
}

void QvisViewWindow::MouseRelease(const QPointF &point)
{
//    std::cerr << __FUNCTION__ << "  " << __LINE__  << "  " << mActiveSubWindowIndex << "  "
//              << point.x() << "  " << point.y() << "  "
//              << std::endl;
}

void QvisViewWindow::MouseMove(const QPointF &point)
{
//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << mActiveSubWindowIndex << "  "
//              << point.x() << "  " << point.y() << "  "
//              << std::endl;
}

void QvisViewWindow::CaptureButtonClicked()
{
    QString message("From : " + this->windowTitle() + " captured: ");

    if(mLayoutMode != QvisViewWindow::LayoutSingle)
        message += "\n";

    QString filenames;

    // Get the full file name to be used.
    QString basename(mCapturePreferencesAttributes->getFullName().c_str());

    for(const auto & index : mViewLayoutWindows[mLayoutMode])
    {
        QString filename = basename;

        // When there are multiple view windows save each one with a character extension.
        if(mLayoutMode != QvisViewWindow::LayoutSingle)
            filename.insert(basename.size()-4, "_" + mViewLayoutExtension[index]);

        std::ofstream ofs(filename.toStdString());
        ofs.close();

        filenames += filename;

        if(mLayoutMode != QvisViewWindow::LayoutSingle)
            filenames += "\n";
    }

    emit postInformationalMessage(message + filenames);
}

void QvisViewWindow::updateID(int id)
{
    mID = id;

    this->setWindowTitle(QString("View Window %1").arg(mID));
    this->setObjectName(this->windowTitle().simplified().remove(" "));

    if(mNumViewWindows <= mID)
        mNumViewWindows = mID + 1;
}

void QvisViewWindow::setCapturePreferencesAttributes(CapturePreferencesAttributes *atts)
{
    mCapturePreferencesAttributes = atts;
}

void QvisViewWindow::setViewWindowActionList(QList<QAction *> & list)
{
    mViewWindowActionList = list;
}

void QvisViewWindow::update()
{
    // Trigger an update to all of the open gl widgets.
    switch(mLayoutMode)
    {
    case QvisViewWindow::LayoutSingle:
        mOpenGLWidget[0]->update();
        break;

    case QvisViewWindow::LayoutVertical:
        mOpenGLWidget[0]->update();
        mOpenGLWidget[1]->update();
        break;

    case QvisViewWindow::LayoutHorizontal:
        mOpenGLWidget[0]->update();
        mOpenGLWidget[2]->update();
        break;

    case QvisViewWindow::LayoutQuad:
        mOpenGLWidget[0]->update();
        mOpenGLWidget[1]->update();
        mOpenGLWidget[2]->update();
        mOpenGLWidget[3]->update();
        break;

    default:
        break;
    }
}

void QvisViewWindow::EnterFullScreenMode()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << this->windowTitle().toStdString() << "  "
              << mContextSubWindowIndex << "  " << mFullScreenSubWindowIndex << "  " << mSplitterMode << std::endl;

    // Check for a valid index. Note that the context subwindow index is used.
    if(mContextSubWindowIndex < 0 || mNumSubWindows <= mContextSubWindowIndex ||
       // Check to see if a window is already in full scree - only one is allowed.
       (0 <= mFullScreenSubWindowIndex && mFullScreenSubWindowIndex < mNumSubWindows))
        return;

    // Save the index so the subwindow gets restored and prevents another subwindow from becoming full screen.
    mFullScreenSubWindowIndex = mContextSubWindowIndex;

    // When the splitters are fixed the subwindow should be removed before placing into the dialog.
    if(mSplitterMode == QvisViewWindow::Fixed)
        ui->OpenGLGridLayout->removeWidget(mOpenGLWidget[mFullScreenSubWindowIndex]);
    // When the splitters are movable the splitter knows the subwindow was removed.
    else //if(mSplitterMode == QvisViewWindow::Movable)
    {
        // When items are removed from splitters, the splitter
        // state may not be preserved so store the state.
        mMainSplitterData   = ui->MainSplitter  ->saveState();
        mTopSplitterData    = ui->TopSplitter   ->saveState();
        mBottomSplitterData = ui->BottomSplitter->saveState();
    }

    // Create a new dialog that contains the subwindow.
    mViewWindowDialog = new QvisViewWindowDialog(this);
    mViewWindowDialog->setWidget(mOpenGLWidget[mFullScreenSubWindowIndex]);
    // Add a connection so that view window knows when the dialog exits full screen
    // so that the subwindow can be restored.
    QObject::connect(mViewWindowDialog, SIGNAL(exitFullScreen()), this, SLOT(ExitFullScreenMode()));

    // Show the window in full screen.
    mViewWindowDialog->showFullScreen();
}

void QvisViewWindow::ExitFullScreenMode()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << this->windowTitle().toStdString() << "  " << mFullScreenSubWindowIndex << "  " << mSplitterMode << std::endl;

    // Check for a valid index.
    if(mFullScreenSubWindowIndex < 0 || mNumSubWindows <= mFullScreenSubWindowIndex)
        return;

    // Remove the subwindow from the dialog
    mViewWindowDialog->removeWidget(mOpenGLWidget[mFullScreenSubWindowIndex]);

    // When fixed the row amd colum are know based on the index.
    if(mSplitterMode == QvisViewWindow::Fixed)
    {
        int row = mFullScreenSubWindowIndex / 2;
        int col = mFullScreenSubWindowIndex % 2;

        ui->OpenGLGridLayout->addWidget(mOpenGLWidget[mFullScreenSubWindowIndex], row, col, 1, 1);
    }
    // When moveable the subwindow must be inserted back into the correct splitter at
    // the correct location.
    else //if(mSplitterMode == QvisViewWindow::Movable)
    {
        switch(mFullScreenSubWindowIndex)
        {
        case 0:
            ui->TopSplitter   ->insertWidget(0, mOpenGLWidget[0]);
            break;
        case 1:
            ui->TopSplitter   ->insertWidget(1, mOpenGLWidget[1]);
            break;
        case 2:
            ui->BottomSplitter->insertWidget(0, mOpenGLWidget[2]);
            break;
        case 3:
            ui->BottomSplitter->insertWidget(1, mOpenGLWidget[3]);
            break;
        }

        // After adding the items restore the splitter state.
        ui->MainSplitter  ->restoreState(mMainSplitterData);
        ui->TopSplitter   ->restoreState(mTopSplitterData);
        ui->BottomSplitter->restoreState(mBottomSplitterData);
    }

    // Delete the dialog and reset the index.
    mViewWindowDialog->setParent(nullptr);
    mViewWindowDialog->deleteLater();
    mViewWindowDialog = nullptr;

    mContextSubWindowIndex = -1;
    mFullScreenSubWindowIndex = -1;
}
