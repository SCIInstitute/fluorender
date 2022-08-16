#include "QvisPaintBrushDialog.h"
#include "ui_QvisPaintBrushDialog.h"

#include "QvisDragDropToolBar.h"

#include <iostream>

QvisPaintBrushDialog::QvisPaintBrushDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisPaintBrushDialog)
{
    ui->setupUi(this);

    // The toolbar allows the user to quickly access paint functions.
    dragDropToolBar();
    dragDropToolBar()->setIcon(this->windowIcon());
    dragDropToolBar()->setMaxColumns(10);

    ui->MainLayout->insertWidget(0, dragDropToolBar()->dragToolBar());

    // Move the tool buttons which were created in Qt Designer to the drag and drop tool bar.
    // First row
    addDragAction(ui->UndoButton,     SLOT(UndoButtonClicked()));
    addDragAction(ui->RedoButton,     SLOT(RedoButtonClicked()));
    addDragAction(ui->GrowButton,     SLOT(GrowButtonClicked()),     true);
    addDragAction(ui->SelectButton,   SLOT(SelectButtonClicked()),   true);
    addDragAction(ui->DifuseButton,   SLOT(DifuseButtonClicked()));
    addDragAction(ui->SolidButton,    SLOT(SolidButtonClicked()));
    addDragAction(ui->UndoButton,     SLOT(UndoButtonClicked()));
    addDragAction(ui->UnselectButton, SLOT(UnselectButtonClicked()), true);
    addDragAction(ui->EraseButton,    SLOT(EraseButtonClicked()),    true);
    addDragAction(ui->ExtractButton,  SLOT(ExtractButtonClicked()));
    addDragAction(ui->ResetButton,    SLOT(ResetButtonClicked()));

    // Second row
    addDragAction(ui->CopyButton,      SLOT(CopyButtonClicked()));
    addDragAction(ui->DataButton,      SLOT(DataButtonClicked()));
    addDragAction(ui->PasteButton,     SLOT(PasteButtonClicked()));
    addDragAction(ui->MergeButton,     SLOT(MergeButtonClicked()));
    addDragAction(ui->ExcludeButton,   SLOT(ExcludeButtonClicked()));
    addDragAction(ui->IntersectButton, SLOT(IntersectButtonClicked()));

    ui->ToolbarLine->setHidden(true);
}

QvisPaintBrushDialog::~QvisPaintBrushDialog()
{
    delete ui;
}

void QvisPaintBrushDialog::updateWindow(bool doAll)
{

}

// First row
void QvisPaintBrushDialog::UndoButtonClicked()
{
}

void QvisPaintBrushDialog::RedoButtonClicked()
{
}

void QvisPaintBrushDialog::GrowButtonClicked()
{
}

void QvisPaintBrushDialog::SelectButtonClicked()
{
}

void QvisPaintBrushDialog::DifuseButtonClicked()
{
}

void QvisPaintBrushDialog::SolidButtonClicked()
{
}

void QvisPaintBrushDialog::UnselectButtonClicked()
{
}

void QvisPaintBrushDialog::EraseButtonClicked()
{
}

void QvisPaintBrushDialog::ExtractButtonClicked()
{
}

void QvisPaintBrushDialog::ResetButtonClicked()
{
}

// Second row
void QvisPaintBrushDialog::CopyButtonClicked()
{
}

void QvisPaintBrushDialog::DataButtonClicked()
{
}

void QvisPaintBrushDialog::PasteButtonClicked()
{
}

void QvisPaintBrushDialog::MergeButtonClicked()
{
}

void QvisPaintBrushDialog::ExcludeButtonClicked()
{
}

void QvisPaintBrushDialog::IntersectButtonClicked()
{
}

// Selection Settings
void QvisPaintBrushDialog::AutoClearToggled(bool checked)
{

}

void QvisPaintBrushDialog::EdgeDetectToggled(bool checked)
{

}

void QvisPaintBrushDialog::VisibleOnlyToggled(bool checked)
{

}

void QvisPaintBrushDialog::CrossBricksToggled(bool checked)
{

}

void QvisPaintBrushDialog::ApplyToGroupToggled(bool checked)
{

}

void QvisPaintBrushDialog::ThresholdSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ThresholdSpinBox);
    ui->ThresholdSpinBox->setValue(double(value)/100.0);
    ui->ThresholdSpinBox->setFocus();

    sendNotification();
}

void QvisPaintBrushDialog::ThresholdValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ThresholdSlider);
    ui->ThresholdSlider->setValue(value*100.0);

    sendNotification();
}

void QvisPaintBrushDialog::EdgeStrengthSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->EdgeStrengthSpinBox);
    ui->EdgeStrengthSpinBox->setValue(double(value)/100.0);
    ui->EdgeStrengthSpinBox->setFocus();

    sendNotification();
}

void QvisPaintBrushDialog::EdgeStrengthValueChanged(double value)
{
    const QSignalBlocker blocker(ui->EdgeStrengthSlider);
    ui->EdgeStrengthSlider->setValue(value*100.0);

    sendNotification();
}

// Brush Properties
void QvisPaintBrushDialog::GrowthCurrentIndexChanged(int index)
{

}

void QvisPaintBrushDialog::DependentCurrentIndexChanged(int index)
{

}

void QvisPaintBrushDialog::CenterSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->CenterSizeSpinBox);
    ui->CenterSizeSpinBox->setValue(value);
    ui->CenterSizeSpinBox->setFocus();

    sendNotification();
}

void QvisPaintBrushDialog::CenterSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->CenterSizeSlider);
    ui->CenterSizeSlider->setValue(value);

    sendNotification();
}

void QvisPaintBrushDialog::GrowSizeToggled(bool checked)
{
    ui->GrowSizeSlider->setEnabled(checked);
    ui->GrowSizeSpinBox->setEnabled(checked);
}

void QvisPaintBrushDialog::GrowSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->GrowSizeSpinBox);
    ui->GrowSizeSpinBox->setValue(value);
    ui->GrowSizeSpinBox->setFocus();

    sendNotification();
}

void QvisPaintBrushDialog::GrowSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->GrowSizeSlider);
    ui->GrowSizeSlider->setValue(value);

    sendNotification();
}

// Align
void QvisPaintBrushDialog::TriAxisXYZButtonClicked()
{

}

void QvisPaintBrushDialog::TriAxisXZYButtonClicked()
{

}

void QvisPaintBrushDialog::TriAxisYXZButtonClicked()
{

}

void QvisPaintBrushDialog::TriAxisYZXButtonClicked()
{

}

void QvisPaintBrushDialog::TriAxisZXYButtonClicked()
{

}

void QvisPaintBrushDialog::TriAxisZYXButtonClicked()
{

}

void QvisPaintBrushDialog::MoveToCenterToggled(bool checked)
{

}

// Output
void QvisPaintBrushDialog::GetSelectedSizeButtonClicked()
{

}

void QvisPaintBrushDialog::AutoUpdateToggled(bool checked)
{

}

void QvisPaintBrushDialog::HoldHistoryToggled(bool checked)
{

}

void QvisPaintBrushDialog::HoldHistoryValueChanged(int value)
{

}

void QvisPaintBrushDialog::ClearHistoryButtonClicked()
{
    while( ui->SelectionTableWidget->rowCount() )
      ui->SelectionTableWidget->removeRow( ui->SelectionTableWidget->rowCount() - 1 );
}

