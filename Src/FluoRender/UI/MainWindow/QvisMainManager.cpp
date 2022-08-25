#include "QvisMainManager.h"
#include "ui_QvisMainManager.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>

#include <QvisWorkspaceManager.h>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>

// QvisMainManager
QvisMainManager::QvisMainManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisMainManager)
{
    ui->setupUi(this);

    ui->DatasetManager->setWorkspaceManager(ui->WorkspaceManager);
    ui->WorkspaceManager->setDatasetManager(ui->DatasetManager);

    // Singals from the Dataset Manager that modify the Workspace Manager.
    QObject::connect(ui->DatasetManager, SIGNAL(workspaceUpdateSelection(QString,DatasetAttributes::DatasetType)),
                     ui->WorkspaceManager, SLOT(WorkspaceUpdateSelection(QString,DatasetAttributes::DatasetType)));
    QObject::connect(ui->DatasetManager, SIGNAL(workspaceDelete(QString,DatasetAttributes::DatasetType,QString)),
                     ui->WorkspaceManager, SLOT(WorkspaceDelete(QString,DatasetAttributes::DatasetType,QString)));
    QObject::connect(ui->DatasetManager, SIGNAL(workspaceRename(QString,QString,DatasetAttributes::DatasetType,QString)),
                     ui->WorkspaceManager, SLOT(WorkspaceRename(QString,QString,DatasetAttributes::DatasetType,QString)));
    QObject::connect(ui->DatasetManager, SIGNAL(workspaceAdd(QString,QString,DatasetAttributes::DatasetType,QString)),
                     ui->WorkspaceManager, SLOT(WorkspaceAdd(QString,QString,DatasetAttributes::DatasetType,QString)));
}

QvisMainManager::~QvisMainManager()
{
    delete ui;
}

QByteArray QvisMainManager::saveState(int version) const
{
    QByteArray xmldata;
    QXmlStreamWriter s(&xmldata);

    s.writeStartDocument();
        s.writeStartElement("QvisMainManager");
        s.writeAttribute("Version", QString::number(mCurrentVersion));
        s.writeAttribute("UserVersion", QString::number(version));

        s.writeEndElement();
    s.writeEndDocument();

    return xmldata;
}

bool QvisMainManager::restoreState(const QByteArray &state, int version)
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

    if (s.name() != tr("QvisMainManager"))
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

void QvisMainManager::saveState(QSettings &Settings) const
{
    //Settings.setValue("QvisMainManager", this->saveState());

    Settings.setValue("QvisMainManager/MainSplitter", ui->MainSplitter->saveState());
}

void QvisMainManager::restoreState(const QSettings &Settings)
{
    //this->restoreState(Settings.value("QvisMainManager").toByteArray());

    ui->MainSplitter->restoreState(Settings.value("QvisMainManager/MainSplitter").toByteArray());
}

QvisSourceManager * QvisMainManager::getSourceManager() const
{
    return ui->SourceManager;
}

QvisDatasetManager * QvisMainManager::getDatasetManager() const
{
    return ui->DatasetManager;
}

QvisWorkspaceManager * QvisMainManager::getWorkspaceManager() const
{
    return ui->WorkspaceManager;
}

QvisAnimationController * QvisMainManager::getAnimationController() const
{
    return ui->AnimationController;
}
