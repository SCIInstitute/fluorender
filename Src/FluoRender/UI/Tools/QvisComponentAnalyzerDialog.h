#ifndef QVISCOMPONENTANALYZERDIALOG_H
#define QVISCOMPONENTANALYZERDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisComponentAnalyzerDialog;
}

class QvisComponentAnalyzerDialog : public QvisDialogBase
{
    Q_OBJECT

    enum ClusterMethod {KMEANS, EM, DBSCAN};
public:
    explicit QvisComponentAnalyzerDialog(QWidget *parent = nullptr);
    ~QvisComponentAnalyzerDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ShuffleButtonClicked();
    void IncludeButtonClicked();
    void ExcludeButtonClicked();
    void ClearHistoryButtonClicked();
    void HoldHistoryToggled(bool checked);
    // Generate tab
    void IterationsSliderChanged(int value);
    void IterationsValueChanged(int value);
    void ThresholdSliderChanged(int value);
    void ThresholdValueChanged(double value);
    void DifusionToggled(bool checked);
    void DifusionSmoothnessSliderChanged(int value);
    void DifusionSmoothnessValueChanged(double value);
    void DensityFieldToggled(bool checked);
    void DensityFieldSeparationSliderChanged(int value);
    void DensityFieldSeparationValueChanged(double value);
    void DensityFieldNoiseLevelSliderChanged(int value);
    void DensityFieldNoiseLevelValueChanged(double value);
    void DensityFieldFilterSizeSliderChanged(int value);
    void DensityFieldFilterSizeValueChanged(int value);
    void DensityFieldFeatureSizeSliderChanged(int value);
    void DensityFieldFeatureSizeValueChanged(int value);
    void DistanceFieldToggled(bool checked);
    void DistanceFieldStrengthSliderChanged(int value);
    void DistanceFieldStrengthValueChanged(double value);
    void DistanceFieldPerimeterSliderChanged(int value);
    void DistanceFieldPerimeterValueChanged(double value);
    void DistanceFieldFilterSizeSliderChanged(int value);
    void DistanceFieldFilterSizeValueChanged(int value);
    void DistanceFieldFeatureSizeSliderChanged(int value);
    void DistanceFieldFeatureSizeValueChanged(int value);
    void FixateGrowthToggled(bool checked);
    void FixateGrowthStopSizeSliderChanged(int value);
    void FixateGrowthStopSizeValueChanged(int value);
    void FixateGrowthRefixButtonClicked();
    void PostCleanupToggled(bool checked);
    void PostCleanupIterationsSliderChanged(int value);
    void PostCleanupIterationsValueChanged(int value);
    void PostCleanupStopSizeSliderChanged(int value);
    void PostCleanupStopSizeValueChanged(int value);
    void PostCleanupCleanMoreButtonClicked();
    void MacroRecordTextEditFinished();
    void MacroRecordButtonClicked();
    void MacroPlayButtonClicked();
    void MacroResetButtonClicked();
    void MacroSaveFileTextEditFinished();
    void MacroLoadButtonClicked();
    void MacroSaveButtonClicked();
    void GenerateButtonClicked();
    void GenerateAutoUpdateButtonClicked();
    void GenerateUseSelectionToggled(bool checked);
    // Cluster tab
    void ClusterMethodButtonClicked(int index);
    void MethodKMeansButtonClicked();
    void MethodEMButtonClicked();
    void MethodDBSCANButtonClicked();
    void SettingsCusterNumberSliderChanged(int value);
    void SettingsCusterNumberValueChanged(int value);
    void SettingsToleranceSliderChanged(int value);
    void SettingsToleranceValueChanged(double value);
    void SettingsMinimumSizeSliderChanged(int value);
    void SettingsMinimumSizeValueChanged(int value);
    void SettingsNeighborhoodSliderChanged(int value);
    void SettingsNeighborhoodValueChanged(double value);
    void SettingsMaxIterationsSliderChanged(int value);
    void SettingsMaxIterationsValueChanged(int value);
    void ClusterButtonClicked();
    // Analysis tab
    void SelectionIDTextEditFinished();
    void SelectionIDMinimumToggled(bool checked);
    void SelectionIDMinimumValueChanged(int value);
    void SelectionIDMaximumToggled(bool checked);
    void SelectionIDMaximumValueChanged(int value);
    void SelectionAppendButtonClicked();
    void SelectionExclusiveButtonClicked();
    void SelectionAllVoxelsButtonClicked();
    void SelectionFullfilButtonClicked();
    void SelectionClearButtonClicked();
    void ModifyIDTextEditFinished();
    void ModifyAssignButtonClicked();
    void ModifyAddButtonClicked();
    void ModifyReplaceButtonClicked();
    void ModifyCleanSelectionButtonClicked();
    void ModifyCombineButtonClicked();
    void ModifyOptionsContactSizeSliderChanged(int value);
    void ModifyOptionsContactSizeValueChanged(int value);
    void ModifyOptionsConsistentColorToggled(bool checked);
    void ModifyOptionsComputeColocalizationToggled(bool checked);
    void ChannelTypeEachComponentButtonClicked();
    void ChannelTypeButtonClicked(int index);
    void ChannelTypeRGBButtonClicked();
    void ChannelOutputRandonColorsButtonClicked();
    void ChannelOutputSizeBasedButtonClicked();
    void ChannelOutputIDButtonClicked();
    void ChannelOutputSerialNumberButtonClicked();
    void DistancesFilterNearestNeighborsToggled(bool checked);
    void DistancesFilterNearestNeighborsSliderChanged(int value);
    void DistancesFilterNearestNeighborsValueChanged(int value);
    void DistancesAllChannelResultsToggled(bool checked);
    void DistancesComputeButtonClicked();
    void XYZButtonClicked();
    void YXZButtonClicked();
    void ZXYButtonClicked();
    void XZYButtonClicked();
    void YZXButtonClicked();
    void ZYXButtonClicked();
    void MoveToCenterToggled(bool checked);
    void AnalyzeButtonClicked();
    void AnalyzeSelectionButtonClicked();

private:
    Ui::QvisComponentAnalyzerDialog *ui;

    void SetClusterMethod( ClusterMethod method );
};

#endif // QVISCOMPONENTANALYZERDIALOG_H
