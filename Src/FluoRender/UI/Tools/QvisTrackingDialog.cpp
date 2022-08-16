#include "QvisTrackingDialog.h"
#include "ui_QvisTrackingDialog.h"

QvisTrackingDialog::QvisTrackingDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisTrackingDialog)
{
    ui->setupUi(this);
}

QvisTrackingDialog::~QvisTrackingDialog()
{
    delete ui;
}

void QvisTrackingDialog::updateWindow(bool doAll)
{

}

void QvisTrackingDialog::resizeEvent(QResizeEvent *event)
{
    // Reset the header column width so the header stays centered below the buttons.
    auto width = int(ui->IDListsTableWidget->geometry().width() / ui->IDListsTableWidget->columnCount());

    if(width < 60)
        width = 60;
    ui->IDListsTableWidget->horizontalHeader()->setDefaultSectionSize(width);

    QWidget::resizeEvent(event);
}

// Main
void QvisTrackingDialog::TracksTailToggled(bool checked)
{
    // Tail or lead turns on the slider
    checked |= ui->TracksLeadCheckBox->isChecked();

    ui->TracksSlider->setEnabled(checked);
    ui->TracksSpinBox->setEnabled(checked);
}

void QvisTrackingDialog::TracksLeadToggled(bool checked)
{
    // Tail or lead turns on the slider
    checked |= ui->TracksTailCheckBox->isChecked();

    ui->TracksSlider->setEnabled(checked);
    ui->TracksSpinBox->setEnabled(checked);
}

void QvisTrackingDialog::TracksSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->TracksSpinBox);
    ui->TracksSpinBox->setValue(value);
    ui->TracksSpinBox->setFocus();
}

void QvisTrackingDialog::TracksValueChanged(int value)
{
    const QSignalBlocker blocker(ui->TracksSlider);
    ui->TracksSlider->setValue(value);
}

void QvisTrackingDialog::IDListsBackwardButtonClicked()
{

}

void QvisTrackingDialog::IDListsForewardButtonClicked()
{

}

// Track map
void QvisTrackingDialog::TrackMapTextEditFinished()
{

}

void QvisTrackingDialog::TrackMapLoadButtonClicked()
{

}

void QvisTrackingDialog::TrackMapSaveButtonClicked()
{

}

void QvisTrackingDialog::TrackMapSaveAsButtonClicked()
{

}

void QvisTrackingDialog::TrackMapIterationsValueChanged(int value)
{

}

void QvisTrackingDialog::TrackMapSizeTresholdValueChanged(int value)
{

}

void QvisTrackingDialog::TrackMapSimilarityValueChanged(double value)
{

}


void QvisTrackingDialog::TrackMapContractFactorValueChanged(double value)
{

}

void QvisTrackingDialog::TrackMapConsistentColorToggled(bool checked)
{

}

void QvisTrackingDialog::TrackMapTryMergingToggled(bool checked)
{

}

void QvisTrackingDialog::TrackMapTrySplittingToggled(bool checked)
{

}

void QvisTrackingDialog::TrackMapGenerateButtonClicked()
{

}

void QvisTrackingDialog::TrackMapRefineTButtonClicked()
{

}

void QvisTrackingDialog::TrackMapRefineAllButtonClicked()
{

}

// Selection
void QvisTrackingDialog::SelectionTextEditFinished()
{

}

void QvisTrackingDialog::SelectionAppendButtonClicked()
{

}

void QvisTrackingDialog::SelectionClearButtonClicked()
{

}

void QvisTrackingDialog::SelectionReplaceButtonClicked()
{

}

void QvisTrackingDialog::SelectionFullComponentButtonClicked()
{

}

void QvisTrackingDialog::SelectionShuffleButtonClicked()
{

}

void QvisTrackingDialog::SelectionComponentSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SelectionComponentSizeSpinBox);
    ui->SelectionComponentSizeSpinBox->setValue(value);
    ui->SelectionComponentSizeSpinBox->setFocus();
}

void QvisTrackingDialog::SelectionComponentSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SelectionComponentSizeSlider);
    ui->SelectionComponentSizeSlider->setValue(value);
}

void QvisTrackingDialog::SelectionUncertaintySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SelectionUncertaintySpinBox);
    ui->SelectionUncertaintySpinBox->setValue(value);
    ui->SelectionUncertaintySpinBox->setFocus();
}

void QvisTrackingDialog::SelectionUncertaintyValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SelectionUncertaintySlider);
    ui->SelectionUncertaintySlider->setValue(value);
}

void QvisTrackingDialog::SelectionUncertaintyButtonClicked()
{

}

// Modify
void QvisTrackingDialog::ModifySelectionTextEditFinished()
{

}

void QvisTrackingDialog::ModifyAppendButtonClicked()
{

}

void QvisTrackingDialog::ModifyClearButtonClicked()
{

}

void QvisTrackingDialog::ModifyAssignIDButtonClicked()
{

}

void QvisTrackingDialog::ModifyAddIDButtonClicked()
{

}

void QvisTrackingDialog::ModifyReplaceIDButtonClicked()
{

}

void QvisTrackingDialog::ModifyCombineButtonClicked()
{

}

void QvisTrackingDialog::ModifySeparateButtonClicked()
{

}

void QvisTrackingDialog::ModifySegmentButtonClicked()
{

}

void QvisTrackingDialog::ModifyNumberOfComponentsValueChanged(int value)
{

}

// Linkage
void QvisTrackingDialog::LinkageSelectionTextEditFinished()
{

}

void QvisTrackingDialog::LinkageAppendButtonClicked()
{

}

void QvisTrackingDialog::LinkageClearButtonClicked()
{

}

void QvisTrackingDialog::LinkExclusiveButtonClicked()
{

}

void QvisTrackingDialog::LinkIDsButtonClicked()
{

}

void QvisTrackingDialog::LinkNewIDsButtonClicked()
{

}

void QvisTrackingDialog::LinkIsolateButtonClicked()
{

}

void QvisTrackingDialog::UnlinkIDsButtonClicked()
{

}

// Analysis
void QvisTrackingDialog::AnalysisRulersButtonClicked()
{

}

void QvisTrackingDialog::AnalysisUniformIDsButtonClicked()
{

}

void QvisTrackingDialog::AnalysisComputeButtonClicked()
{

}

void QvisTrackingDialog::AnalysisLinksButtonClicked()
{

}

void QvisTrackingDialog::AnalysisUncertaintyButtonClicked()
{

}

void QvisTrackingDialog::AnalysisPathsButtonClicked()
{

}

void QvisTrackingDialog::AnalysisExportButtonClicked()
{

}

