#include "QvisMeasurementDialog.h"
#include "ui_QvisMeasurementDialog.h"

#include "QvisDragDropToolBar.h"
#include <iostream>

QvisMeasurementDialog::QvisMeasurementDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisMeasurementDialog)
{
    ui->setupUi(this);

    // The toolbar allows the user to quickly access paint functions.
    dragDropToolBar();
    dragDropToolBar()->setIcon(this->windowIcon());
    dragDropToolBar()->setMaxColumns(8);

    ui->MainLayout->insertWidget(0, dragDropToolBar()->dragToolBar());

    // Move the tool buttons which were created in Qt Designer to the drag and drop tool bar.
    // First row
    addDragAction(ui->LocatorButton,        SLOT(LocatorButtonClicked()),        true);
    addDragAction(ui->ProbeButton,          SLOT(ProbeButtonClicked()),          true);
    addDragAction(ui->LineButton,           SLOT(LineButtonClicked()),           true);
    addDragAction(ui->ProtractorButton,     SLOT(ProtractorButtonClicked()),     true);
    addDragAction(ui->EllipseButton,        SLOT(EllipseButtonClicked()),        true);
    addDragAction(ui->MultiPointLineButton, SLOT(MultiPointLineButtonClicked()), true);
    addDragAction(ui->PencilButton,         SLOT(PencilButtonClicked()),         true);
    addDragAction(ui->GrowButton,           SLOT(GrowButtonClicked()));
    // Second row
    addDragAction(ui->MoveButton,    SLOT(MoveButtonClicked()));
    addDragAction(ui->EditButton,    SLOT(EditButtonClicked()));
    addDragAction(ui->DeleteButton,  SLOT(DeleteButtonClicked()));
    addDragAction(ui->FlipButton,    SLOT(FlipButtonClicked()));
    addDragAction(ui->AverageButton, SLOT(AverageButtonClicked()));
    addDragAction(ui->PruneButton,   SLOT(PruneButtonClicked()));
    addDragAction(ui->LockButton,    SLOT(LockButtonClicked()));
    addDragAction(ui->RelaxButton,   SLOT(RelaxButtonClicked()));
    // Third row.
    addDragAction(ui->DeleteRulerButton,     SLOT(DeleteRulerButtonClicked()));
    addDragAction(ui->DeleteAllRulersButton, SLOT(DeleteAllRulersButtonClicked()));
    addDragAction(ui->ProfileButton,         SLOT(ProfileButtonClicked()));
    addDragAction(ui->DistanceButton,        SLOT(DistanceButtonClicked()));
    addDragAction(ui->ProjectButton,         SLOT(ProjectButtonClicked()));
    addDragAction(ui->ExportButton,          SLOT(ExportButtonClicked()));

    ui->ToolLine->setHidden(true);
}

QvisMeasurementDialog::~QvisMeasurementDialog()
{
    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << windowTitle().toStdString() << std::endl;

    delete ui;
}

void QvisMeasurementDialog::updateWindow(bool doAll)
{

}

// First row
void QvisMeasurementDialog::LocatorButtonClicked()
{
}

void QvisMeasurementDialog::ProbeButtonClicked()
{
}

void QvisMeasurementDialog::LineButtonClicked()
{
}

void QvisMeasurementDialog::ProtractorButtonClicked()
{
}

void QvisMeasurementDialog::EllipseButtonClicked()
{
}

void QvisMeasurementDialog::MultiPointLineButtonClicked()
{
}

void QvisMeasurementDialog::PencilButtonClicked()
{
}

void QvisMeasurementDialog::GrowButtonClicked()
{
}

// Second row
void QvisMeasurementDialog::MoveButtonClicked()
{
}

void QvisMeasurementDialog::EditButtonClicked()
{
}

void QvisMeasurementDialog::DeleteButtonClicked()
{
}

void QvisMeasurementDialog::FlipButtonClicked()
{
}

void QvisMeasurementDialog::AverageButtonClicked()
{
}

void QvisMeasurementDialog::PruneButtonClicked()
{
}

void QvisMeasurementDialog::LockButtonClicked()
{
}

void QvisMeasurementDialog::RelaxButtonClicked()
{
}

// Third row
void QvisMeasurementDialog::DeleteRulerButtonClicked()
{
}

void QvisMeasurementDialog::DeleteAllRulersButtonClicked()
{
}

void QvisMeasurementDialog::ProfileButtonClicked()
{
}

void QvisMeasurementDialog::DistanceButtonClicked()
{
}

void QvisMeasurementDialog::ProjectButtonClicked()
{
}

void QvisMeasurementDialog::ExportButtonClicked()
{
}

// Settings
void QvisMeasurementDialog::ZDepthViewPlaneButtonClicked()
{

}

void QvisMeasurementDialog::ZDepthMaxIntensityButtonClicked()
{

}

void QvisMeasurementDialog::ZDepthAccumIntensityButtonClicked()
{

}

void QvisMeasurementDialog::TransientToggled(bool checked)
{

}

void QvisMeasurementDialog::UseVolumePropertiesToggled(bool checked)
{

}

void QvisMeasurementDialog::ComputeDeltaFFToggled(bool checked)
{

}

void QvisMeasurementDialog::AutoRelaxToggled(bool checked)
{

}

void QvisMeasurementDialog::ConstraintIndexChanged(int index)
{

}

void QvisMeasurementDialog::ExInRatioValueChanged(double value)
{

}

// Ruler list
void QvisMeasurementDialog::NewGroupButtonClicked()
{

}

void QvisMeasurementDialog::GroupIDValueChanged(int value)
{

}

void QvisMeasurementDialog::ChangeGroupButtonClicked()
{

}

void QvisMeasurementDialog::SelectGroupButtonClicked()
{

}

void QvisMeasurementDialog::DisplayGroupButtonClicked()
{

}

// Align
void QvisMeasurementDialog::MonoAxisXPosButtonClicked()
{

}

void QvisMeasurementDialog::MonoAxisXNegButtonClicked()
{

}

void QvisMeasurementDialog::MonoAxisYPosButtonClicked()
{

}

void QvisMeasurementDialog::MonoAxisYNegButtonClicked()
{

}

void QvisMeasurementDialog::MonoAxisZPosButtonClicked()
{

}

void QvisMeasurementDialog::MonoAxisZNegButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisXYZButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisXZYButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisYXZButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisYZXButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisZXYButtonClicked()
{

}

void QvisMeasurementDialog::TriAxisZYXButtonClicked()
{

}

void QvisMeasurementDialog::MoveToCenterToggled(bool checked)
{

}

