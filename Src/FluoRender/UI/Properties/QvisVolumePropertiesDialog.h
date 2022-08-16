#ifndef QVISVOLUMEPROPERTIESDIALOG_H
#define QVISVOLUMEPROPERTIESDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisVolumePropertiesDialog;
}

class VolumePropertiesAttributes
{
public:

    enum
    {
        // Rendering
        ID_XVoxel = 0,
        ID_YVoxel,
        ID_ZVoxel,
        ID_RenderingMethod,
        ID_ColorSource,
        ID_PrimaryColor,
        ID_SecondaryColor,
        ID_ColorTable,
        ID_ColorDataSource,
        ID_DataMinimum,
        ID_DataMaximum,
        ID_SampleRate,
        ID_HighTransparancyMode,
        ID_ShowComponents,
        ID_SmoothFinalImage,
        ID_InvertDataValues,
        ID_UseInterpolation,
        ID_EnableDepthMode,
        ID_ShowLegend,
        ID_SyncAttributes,
        // Transfer
        ID_Gamma,
        ID_Saturation,
        ID_Luminance,
        ID_AlphaSet,
        ID_Alpha,
        ID_ShadowSet,
        ID_Shadow,
        ID_ExtractBoundary,
        ID_ShadingSet,
        ID_ShadingIntensity,
        ID_ShadingShinniness,
        ID_ShadingThresholdMin,
        ID_ShadingThresholdMax,
        ID_LAST_VARIABLE
    };

    enum RenderingMethod {
        Volume,
        MIP,
    };

    enum ColorSource {
        Solid,
        ColorMap,
    };

    enum ColorTable {
        Rainbox,
        Hot,
        Cool,
        Diverging,
        Monchromo,
        HighKey,
        LowKey,
        HighTransparency,
    };

    enum DataSource {
        Intensity,
        Gradient,
        Differential,
        XValue,
        YValue,
        ZValue,
    };

    bool isSelected(int i) { Q_UNUSED(i); return false; };
    int numAttributes() { return ID_LAST_VARIABLE; };
};

class QvisVolumePropertiesDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisVolumePropertiesDialog(QWidget *parent = nullptr);
    ~QvisVolumePropertiesDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    // Dialog
    void ResetAllButtonClicked();
    void MakeDefaultButtonClicked();
    // Rendering
    void XVoxelValueChanged(double value);
    void YVoxelValueChanged(double value);
    void ZVoxelValueChanged(double value);
    void RenderingMethodCurrentIndexChanged(int index);
    void ColorSourceCurrentIndexChanged(int index);
    void PrimaryColorButtonClicked();
    void SecondaryColorButtonClicked();
    void ColorTableCurrentIndexChanged(int index);
    void ColorDataSourceCurrentIndexChanged(int index);
    void DataMinimumValueChanged(int value);
    void DataRangeSliderLowerChanged(int value);
    void DataRangeSliderUpperChanged(int value);
    void DataMaximumValueChanged(int value);
    void SampleRateSliderChanged(int value);
    void SampleRateValueChanged(double value);
    void HighTransparancyModeToggled(bool checked);
    void ShowComponentsToggled(bool checked);
    void SmoothFinalImageToggled(bool checked);
    void InvertDataValuesToggled(bool checked);
    void UseInterpolationToggled(bool checked);
    void EnableDepthModeToggled(bool checked);
    void ShowLegendToggled(bool checked);
    void SyncAttributesToggled(bool checked);
    // Transfer
    void GammalSliderChanged(int value);
    void GammaValueChanged(double value);
    void SaturationSliderChanged(int value);
    void SaturationValueChanged(int value);
    void LuminanceSliderChanged(int value);
    void LuminanceValueChanged(int value);
    void AlphaToggled(bool checked);
    void AlphaSliderChanged(int value);
    void AlphaValueChanged(int value);
    void ShadowToggled(bool checked);
    void ShadowSliderChanged(int value);
    void ShadowValueChanged(double value);
    void ExtractBoundarySliderChanged(int value);
    void ExtractBoundaryValueChanged(double value);
    void ShadingToggled(bool checked);
    void ShadingIntensityValueChanged(double value);
    void ShadingIntensitySliderChanged(int value);
    void ShadingShinninessValueChanged(double value);
    void ShadingShinninessSliderChanged(int value);
    void ShadingThresholdMinValueChanged(int value);
    void ShadingThresholdMaxValueChanged(int value);
    void ShadingThresholdSliderLowerChanged(int aLowerValue);
    void ShadingThresholdSliderUpperChanged(int aUpperValue);

private:
    Ui::QvisVolumePropertiesDialog *ui;

    void RenderMethodEnable();
    void ColorSourceEnable();

    // Variables
    VolumePropertiesAttributes* mAttributes{nullptr};
};

#endif // QVISVOLUMEPROPERTIESDIALOG_H
