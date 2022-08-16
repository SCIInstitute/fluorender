#include "QvisComponentAnalyzerDialog.h"
#include "ui_QvisComponentAnalyzerDialog.h"

#include <iostream>

QvisComponentAnalyzerDialog::QvisComponentAnalyzerDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisComponentAnalyzerDialog)
{
    ui->setupUi(this);

    this->SetClusterMethod(QvisComponentAnalyzerDialog::KMEANS);
}

QvisComponentAnalyzerDialog::~QvisComponentAnalyzerDialog()
{
    delete ui;
}

void QvisComponentAnalyzerDialog::updateWindow(bool doAll)
{

}

// Main
void QvisComponentAnalyzerDialog::ShuffleButtonClicked()
{

}

void QvisComponentAnalyzerDialog::IncludeButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ExcludeButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ClearHistoryButtonClicked()
{
    while( ui->AnalysisTableWidget->rowCount() )
      ui->AnalysisTableWidget->removeRow( ui->AnalysisTableWidget->rowCount() - 1 );
}

void QvisComponentAnalyzerDialog::HoldHistoryToggled(bool checked)
{

}

// Generate tab
void QvisComponentAnalyzerDialog::IterationsSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->IterationsSpinBox);
    ui->IterationsSpinBox->setValue(value);
    ui->IterationsSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::IterationsValueChanged(int value)
{
    const QSignalBlocker blocker(ui->IterationsSlider);
    ui->IterationsSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::ThresholdSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ThresholdSpinBox);
    ui->ThresholdSpinBox->setValue(value);
    ui->ThresholdSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::ThresholdValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ThresholdSlider);
    ui->ThresholdSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DifusionToggled(bool checked)
{
    ui->DifusionSmoothnessSpinBox->setEnabled(checked);
    ui->DifusionSmoothnessSlider->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::DifusionSmoothnessSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DifusionSmoothnessSpinBox);
    ui->DifusionSmoothnessSpinBox->setValue(value);
    ui->DifusionSmoothnessSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DifusionSmoothnessValueChanged(double value)
{
    const QSignalBlocker blocker(ui->DifusionSmoothnessSlider);
    ui->DifusionSmoothnessSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DensityFieldToggled(bool checked)
{
    ui->DensityFieldSeparationSpinBox->setEnabled(checked);
    ui->DensityFieldSeparationSlider->setEnabled(checked);
    ui->DensityFieldNoiseLevelSpinBox->setEnabled(checked);
    ui->DensityFieldNoiseLevelSlider->setEnabled(checked);
    ui->DensityFieldFilterSizeSpinBox->setEnabled(checked);
    ui->DensityFieldFilterSizeSlider->setEnabled(checked);
    ui->DensityFieldFeatureSizeSpinBox->setEnabled(checked);
    ui->DensityFieldFeatureSizeSlider->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::DensityFieldSeparationSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldSeparationSpinBox);
    ui->DensityFieldSeparationSpinBox->setValue(value);
    ui->DensityFieldSeparationSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DensityFieldSeparationValueChanged(double value)
{
    const QSignalBlocker blocker(ui->DensityFieldSeparationSlider);
    ui->DensityFieldSeparationSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DensityFieldNoiseLevelSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldNoiseLevelSpinBox);
    ui->DensityFieldNoiseLevelSpinBox->setValue(value);
    ui->DensityFieldNoiseLevelSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DensityFieldNoiseLevelValueChanged(double value)
{
    const QSignalBlocker blocker(ui->DensityFieldNoiseLevelSlider);
    ui->DensityFieldNoiseLevelSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DensityFieldFilterSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldFilterSizeSpinBox);
    ui->DensityFieldFilterSizeSpinBox->setValue(value);
    ui->DensityFieldFilterSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DensityFieldFilterSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldFilterSizeSlider);
    ui->DensityFieldFilterSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DensityFieldFeatureSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldFeatureSizeSpinBox);
    ui->DensityFieldFeatureSizeSpinBox->setValue(value);
    ui->DensityFieldFeatureSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DensityFieldFeatureSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DensityFieldFeatureSizeSlider);
    ui->DensityFieldFeatureSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DistanceFieldToggled(bool checked)
{
    ui->DistanceFieldStrengthSpinBox->setEnabled(checked);
    ui->DistanceFieldStrengthSlider->setEnabled(checked);
    ui->DistanceFieldPerimeterSpinBox->setEnabled(checked);
    ui->DistanceFieldPerimeterSlider->setEnabled(checked);
    ui->DistanceFieldFilterSizeSpinBox->setEnabled(checked);
    ui->DistanceFieldFilterSizeSlider->setEnabled(checked);
    ui->DistanceFieldFeatureSizeSpinBox->setEnabled(checked);
    ui->DistanceFieldFeatureSizeSlider->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::DistanceFieldStrengthSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldStrengthSpinBox);
    ui->DistanceFieldStrengthSpinBox->setValue(value);
    ui->DistanceFieldStrengthSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DistanceFieldStrengthValueChanged(double value)
{
    const QSignalBlocker blocker(ui->DistanceFieldStrengthSlider);
    ui->DistanceFieldStrengthSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DistanceFieldPerimeterSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldPerimeterSpinBox);
    ui->DistanceFieldPerimeterSpinBox->setValue(value);
    ui->DistanceFieldFilterSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DistanceFieldPerimeterValueChanged(double value)
{
    const QSignalBlocker blocker(ui->DistanceFieldPerimeterSlider);
    ui->DistanceFieldPerimeterSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DistanceFieldFilterSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldFilterSizeSpinBox);
    ui->DistanceFieldFilterSizeSpinBox->setValue(value);
    ui->DistanceFieldFilterSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DistanceFieldFilterSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldFilterSizeSlider);
    ui->DistanceFieldFilterSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DistanceFieldFeatureSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldFeatureSizeSpinBox);
    ui->DistanceFieldFeatureSizeSpinBox->setValue(value);
    ui->DistanceFieldFeatureSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DistanceFieldFeatureSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DistanceFieldFeatureSizeSlider);
    ui->DistanceFieldFeatureSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::FixateGrowthToggled(bool checked)
{
    ui->FixateGrowthStopSizeSpinBox->setEnabled(checked);
    ui->FixateGrowthStopSizeSlider->setEnabled(checked);
    ui->FixateGrowthRefixButton->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::FixateGrowthStopSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->FixateGrowthStopSizeSpinBox);
    ui->FixateGrowthStopSizeSpinBox->setValue(value);
    ui->FixateGrowthStopSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::FixateGrowthStopSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->FixateGrowthStopSizeSlider);
    ui->FixateGrowthStopSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::FixateGrowthRefixButtonClicked()
{

}

void QvisComponentAnalyzerDialog::PostCleanupToggled(bool checked)
{
    ui->PostCleanupIterationsSpinBox->setEnabled(checked);
    ui->PostCleanupIterationsSlider->setEnabled(checked);
    ui->PostCleanupStopSizeSpinBox->setEnabled(checked);
    ui->PostCleanupStopSizeSlider->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::PostCleanupIterationsSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->PostCleanupIterationsSpinBox);
    ui->PostCleanupIterationsSpinBox->setValue(value);
    ui->PostCleanupIterationsSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::PostCleanupIterationsValueChanged(int value)
{
    const QSignalBlocker blocker(ui->PostCleanupIterationsSlider);
    ui->PostCleanupIterationsSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::PostCleanupStopSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->PostCleanupStopSizeSpinBox);
    ui->PostCleanupStopSizeSpinBox->setValue(value);
    ui->PostCleanupStopSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::PostCleanupStopSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->PostCleanupStopSizeSlider);
    ui->PostCleanupStopSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::PostCleanupCleanMoreButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MacroRecordTextEditFinished()
{

}

void QvisComponentAnalyzerDialog::MacroRecordButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MacroPlayButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MacroResetButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MacroSaveFileTextEditFinished()
{

}

void QvisComponentAnalyzerDialog::MacroLoadButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MacroSaveButtonClicked()
{

}

void QvisComponentAnalyzerDialog::GenerateButtonClicked()
{

}

void QvisComponentAnalyzerDialog::GenerateAutoUpdateButtonClicked()
{

}

void QvisComponentAnalyzerDialog::GenerateUseSelectionToggled(bool checked)
{

}

// Cluster tab
void QvisComponentAnalyzerDialog::ClusterMethodButtonClicked(int index)
{
    std::cerr << "ClusterMethodButtonClicked " << index << std::endl;
}

void QvisComponentAnalyzerDialog::MethodKMeansButtonClicked()
{
    this->SetClusterMethod(QvisComponentAnalyzerDialog::KMEANS);
}

void QvisComponentAnalyzerDialog::MethodEMButtonClicked()
{
    this->SetClusterMethod(QvisComponentAnalyzerDialog::EM);
}

void QvisComponentAnalyzerDialog::MethodDBSCANButtonClicked()
{
    this->SetClusterMethod(QvisComponentAnalyzerDialog::DBSCAN);
}

void QvisComponentAnalyzerDialog::SetClusterMethod( ClusterMethod method )
{
    if(method == QvisComponentAnalyzerDialog::DBSCAN)
    {
        ui->SettingsCusterNumberLabel->hide();
        ui->SettingsCusterNumberSlider->hide();
        ui->SettingsCusterNumberSpinBox->hide();

        ui->SettingsToleranceLabel->show();
        ui->SettingsToleranceSlider->show();
        ui->SettingsToleranceSpinBox->show();

        ui->SettingsMinimumSizeLabel->show();
        ui->SettingsMinimumSizeSlider->show();
        ui->SettingsMinimumSizeSpinBox->show();

        ui->SettingsNeighborhoodLabel->show();
        ui->SettingsNeighborhoodSlider->show();
        ui->SettingsNeighborhoodSpinBox->show();
    }
    else
    {
        ui->SettingsCusterNumberLabel->show();
        ui->SettingsCusterNumberSlider->show();
        ui->SettingsCusterNumberSpinBox->show();

        ui->SettingsToleranceLabel->hide();
        ui->SettingsToleranceSlider->hide();
        ui->SettingsToleranceSpinBox->hide();

        ui->SettingsMinimumSizeLabel->hide();
        ui->SettingsMinimumSizeSlider->hide();
        ui->SettingsMinimumSizeSpinBox->hide();

        ui->SettingsNeighborhoodLabel->hide();
        ui->SettingsNeighborhoodSlider->hide();
        ui->SettingsNeighborhoodSpinBox->hide();
    }
}

void QvisComponentAnalyzerDialog::SettingsCusterNumberSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsCusterNumberSpinBox);
    ui->SettingsCusterNumberSpinBox->setValue(value);
    ui->SettingsCusterNumberSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::SettingsCusterNumberValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsCusterNumberSlider);
    ui->SettingsCusterNumberSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::SettingsToleranceSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsToleranceSpinBox);
    ui->SettingsToleranceSpinBox->setValue(double(value)/100.0);
    ui->SettingsToleranceSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::SettingsToleranceValueChanged(double value)
{
    const QSignalBlocker blocker(ui->SettingsToleranceSlider);
    ui->SettingsToleranceSlider->setValue(value*100.0);
}

void QvisComponentAnalyzerDialog::SettingsMinimumSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsMinimumSizeSpinBox);
    ui->SettingsMinimumSizeSpinBox->setValue(value);
    ui->SettingsMinimumSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::SettingsMinimumSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsMinimumSizeSlider);
    ui->SettingsMinimumSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::SettingsNeighborhoodSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsNeighborhoodSpinBox);
    ui->SettingsNeighborhoodSpinBox->setValue(double(value)/10.0);
    ui->SettingsNeighborhoodSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::SettingsNeighborhoodValueChanged(double value)
{
    const QSignalBlocker blocker(ui->SettingsNeighborhoodSlider);
    ui->SettingsNeighborhoodSlider->setValue(value*10.0);
}

void QvisComponentAnalyzerDialog::SettingsMaxIterationsSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsMaxIterationsSpinBox);
    ui->SettingsMaxIterationsSpinBox->setValue(value);
    ui->SettingsMaxIterationsSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::SettingsMaxIterationsValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SettingsMaxIterationsSlider);
    ui->SettingsMaxIterationsSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::ClusterButtonClicked()
{

}

// Analysis tab

void QvisComponentAnalyzerDialog::SelectionIDTextEditFinished()
{

}

void QvisComponentAnalyzerDialog::SelectionIDMinimumToggled(bool checked)
{
    ui->SelectionIDMinimumSpinBox->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::SelectionIDMinimumValueChanged(int value)
{

}

void QvisComponentAnalyzerDialog::SelectionIDMaximumToggled(bool checked)
{
    ui->SelectionIDMaximumSpinBox->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::SelectionIDMaximumValueChanged(int value)
{

}

void QvisComponentAnalyzerDialog::SelectionAppendButtonClicked()
{

}

void QvisComponentAnalyzerDialog::SelectionExclusiveButtonClicked()
{

}

void QvisComponentAnalyzerDialog::SelectionAllVoxelsButtonClicked()
{

}

void QvisComponentAnalyzerDialog::SelectionFullfilButtonClicked()
{

}

void QvisComponentAnalyzerDialog::SelectionClearButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyIDTextEditFinished()
{

}

void QvisComponentAnalyzerDialog::ModifyAssignButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyAddButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyReplaceButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyCleanSelectionButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyCombineButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ModifyOptionsContactSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ModifyOptionsContactSizeSpinBox);
    ui->ModifyOptionsContactSizeSpinBox->setValue(value);
    ui->ModifyOptionsContactSizeSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::ModifyOptionsContactSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ModifyOptionsContactSizeSlider);
    ui->ModifyOptionsContactSizeSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::ModifyOptionsConsistentColorToggled(bool checked)
{

}

void QvisComponentAnalyzerDialog::ModifyOptionsComputeColocalizationToggled(bool checked)
{

}

void QvisComponentAnalyzerDialog::ChannelTypeEachComponentButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ChannelTypeButtonClicked(int index)
{
    std::cerr << "ChannelTypeButtonClicked " << index << std::endl;
}

void QvisComponentAnalyzerDialog::ChannelTypeRGBButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ChannelOutputRandonColorsButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ChannelOutputSizeBasedButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ChannelOutputIDButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ChannelOutputSerialNumberButtonClicked()
{

}

void QvisComponentAnalyzerDialog::DistancesFilterNearestNeighborsToggled(bool checked)
{
    ui->DistancesFilterNearestNeighborsSpinBox->setEnabled(checked);
    ui->DistancesFilterNearestNeighborsSlider->setEnabled(checked);
}

void QvisComponentAnalyzerDialog::DistancesFilterNearestNeighborsSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DistancesFilterNearestNeighborsSpinBox);
    ui->DistancesFilterNearestNeighborsSpinBox->setValue(value);
    ui->DistancesFilterNearestNeighborsSpinBox->setFocus();
}

void QvisComponentAnalyzerDialog::DistancesFilterNearestNeighborsValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DistancesFilterNearestNeighborsSlider);
    ui->DistancesFilterNearestNeighborsSlider->setValue(value);
}

void QvisComponentAnalyzerDialog::DistancesAllChannelResultsToggled(bool checked)
{

}

void QvisComponentAnalyzerDialog::DistancesComputeButtonClicked()
{

}

void QvisComponentAnalyzerDialog::XYZButtonClicked()
{

}

void QvisComponentAnalyzerDialog::YXZButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ZXYButtonClicked()
{

}

void QvisComponentAnalyzerDialog::XZYButtonClicked()
{

}

void QvisComponentAnalyzerDialog::YZXButtonClicked()
{

}

void QvisComponentAnalyzerDialog::ZYXButtonClicked()
{

}

void QvisComponentAnalyzerDialog::MoveToCenterToggled(bool checked)
{

}

void QvisComponentAnalyzerDialog::AnalyzeButtonClicked()
{

}

void QvisComponentAnalyzerDialog::AnalyzeSelectionButtonClicked()
{

}
