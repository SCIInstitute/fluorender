#include "QvisSourceManager.h"
#include "ui_QvisSourceManager.h"

#include <QFileDialog>

#include <filesystem>
#include <iostream>

#ifdef QT_CREATOR_ONLY
  #include "SourceAgent.h"
#else
  #include "SourceAgent.hpp"
  using namespace fluo;
#endif

// QvisSourceManager
QvisSourceManager::QvisSourceManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisSourceManager)
{
    ui->setupUi(this);

//#ifdef QT_CREATOR_ONLY
    mAgent = new SourceAgent();
//#endif
}

QvisSourceManager::~QvisSourceManager()
{
    delete ui;
}

void QvisSourceManager::SetAgent(InterfaceAgent* agent)
{
    mAgent = (SourceAgent *) agent;
}

// Source buttons
void QvisSourceManager::ProjectOpenClicked(QString filename)
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

    mProjectName = filename;

    QString directory;

    if(mProjectName.lastIndexOf("/") == -1)
        directory = ".";
    else
        directory = mProjectName.left(mProjectName.lastIndexOf("/"));

    emit projectDirectoryChanged(directory);

    mAgent->OpenProject(mProjectName.toStdWString());

    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << filename.toStdString() << "  " << mAgent << std::endl;
}

void QvisSourceManager::ProjectSaveClicked()
{
    // If no projcet then save as.
    if(mProjectName.isEmpty())
    {
        ProjectSaveAsClicked();
    }
    else
    {
        mAgent->SaveProject(mProjectName.toStdWString());
    }
}

void QvisSourceManager::ProjectSaveAsClicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project as"),
                                                    tr(""),
                                                    tr("*.vrp"));

    if(filename.isEmpty())
        return;

    mProjectName = filename;

    QString directory;

    if(filename.lastIndexOf("/") == -1)
        directory = ".";
    else
        directory = mProjectName.left(mProjectName.lastIndexOf("/"));

    emit projectDirectoryChanged(directory);

    mAgent->SaveProject(mProjectName.toStdWString());
}

void QvisSourceManager::VolumeOpenClicked(QString filename)
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

    mAgent->LoadVolume(mProjectName.toStdWString());
}

void QvisSourceManager::MeshOpenClicked(QString filename)
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

    mAgent->LoadMesh(mProjectName.toStdWString());
}
