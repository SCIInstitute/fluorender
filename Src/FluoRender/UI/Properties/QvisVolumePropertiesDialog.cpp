#include "QvisVolumePropertiesDialog.h"
#include "ui_QvisVolumePropertiesDialog.h"

#include <QColorDialog>

QvisVolumePropertiesDialog::QvisVolumePropertiesDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisVolumePropertiesDialog)
{
    ui->setupUi(this);
}

QvisVolumePropertiesDialog::~QvisVolumePropertiesDialog()
{
    delete ui;
}

void QvisVolumePropertiesDialog::updateWindow(bool doAll)
{
    for(int i = 0; i < mAttributes->numAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!mAttributes->isSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
        // Rendering
        case VolumePropertiesAttributes::ID_XVoxel:
        {
            double value = ui->XVoxelSpinBox->value();

            const QSignalBlocker blocker0(ui->XVoxelSpinBox);
            ui->XVoxelSpinBox->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_YVoxel:
        {
            double value = ui->YVoxelSpinBox->value();

            const QSignalBlocker blocker0(ui->YVoxelSpinBox);
            ui->YVoxelSpinBox->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_ZVoxel:
        {
            double value = ui->ZVoxelSpinBox->value();

            const QSignalBlocker blocker0(ui->ZVoxelSpinBox);
            ui->ZVoxelSpinBox->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_RenderingMethod:
        {
            int value = ui->RenderingMethodComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->RenderingMethodComboBox);
            ui->RenderingMethodComboBox->setCurrentIndex(value);

            RenderMethodEnable();
        }
            break;
        case VolumePropertiesAttributes::ID_ColorSource:
        {
            int value = ui->ColorSourceComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->ColorSourceComboBox);
            ui->ColorSourceComboBox->setCurrentIndex(value);

            ColorSourceEnable();
        }
            break;
        case VolumePropertiesAttributes::ID_PrimaryColor:
        {
            QColor color(255,255,255);
            ui->PrimaryColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
            break;
        case VolumePropertiesAttributes::ID_SecondaryColor:
        {
            QColor color(255,255,255);
            ui->SecondaryColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
            break;
        case VolumePropertiesAttributes::ID_ColorTable:
        {
            int value = ui->ColorTableComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->ColorTableComboBox);
            ui->ColorTableComboBox->setCurrentIndex(value);
        }
            break;
        case VolumePropertiesAttributes::ID_ColorDataSource:
        {
            int value = ui->ColorDataSourceComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->ColorDataSourceComboBox);
            ui->ColorDataSourceComboBox->setCurrentIndex(value);
        }
            break;
        case VolumePropertiesAttributes::ID_DataMinimum:
        {
            int value = ui->DataMinimumSpinBox->value();

            const QSignalBlocker blocker0(ui->DataMinimumSpinBox);
            const QSignalBlocker blocker1(ui->DataRangeSlider);

            ui->DataMinimumSpinBox->setValue(value);
            ui->DataRangeSlider->setLowerValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_DataMaximum:
        {
            int value = ui->DataMaximumSpinBox->value();

            const QSignalBlocker blocker0(ui->DataMaximumSpinBox);
            const QSignalBlocker blocker1(ui->DataRangeSlider);

            ui->DataMaximumSpinBox->setValue(value);
            ui->DataRangeSlider->setUpperValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_SampleRate:
        {
            int value = ui->SampleRateSpinBox->value();

            const QSignalBlocker blocker0(ui->SampleRateSpinBox);
            ui->SampleRateSpinBox->setValue(double(value)/100.0);
            const QSignalBlocker blocker1(ui->SampleRateSlider);
            ui->SampleRateSlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_HighTransparancyMode:
        {
            bool checked = ui->HighTransparancyModeCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->HighTransparancyModeCheckBox);
            ui->HighTransparancyModeCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_ShowComponents:
        {
            bool checked = ui->ShowComponentsCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowComponentsCheckBox);
            ui->ShowComponentsCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_SmoothFinalImage:
        {
            bool checked = ui->SmoothFinalImageCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->SmoothFinalImageCheckBox);
            ui->SmoothFinalImageCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_InvertDataValues:
        {
            bool checked = ui->InvertDataValuesCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->InvertDataValuesCheckBox);
            ui->InvertDataValuesCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_UseInterpolation:
        {
            bool checked = ui->UseInterpolationCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->UseInterpolationCheckBox);
            ui->UseInterpolationCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_EnableDepthMode:
        {
            bool checked = ui->EnableDepthModeCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->EnableDepthModeCheckBox);
            ui->EnableDepthModeCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_ShowLegend:
        {
            bool checked = ui->ShowLegendCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowLegendCheckBox);
            ui->ShowLegendCheckBox->setChecked(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_SyncAttributes:
        {
            bool checked = ui->SyncAttributesCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->SyncAttributesCheckBox);
            ui->SyncAttributesCheckBox->setChecked(checked);
        }
            break;
        // Transfer
        case VolumePropertiesAttributes::ID_Gamma:
        {
            int value = ui->GammaSpinBox->value();

            const QSignalBlocker blocker0(ui->GammaSpinBox);
            ui->GammaSpinBox->setValue(double(value)/100.0);
            const QSignalBlocker blocker1(ui->GammaSlider);
            ui->GammaSlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_Saturation:
        {
            int value = ui->SaturationSpinBox->value();

            const QSignalBlocker blocker0(ui->SaturationSpinBox);
            ui->SaturationSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->SaturationSlider);
            ui->SaturationSlider->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_Luminance:
        {
            int value = ui->LuminanceSpinBox->value();

            const QSignalBlocker blocker0(ui->LuminanceSpinBox);
            ui->LuminanceSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->LuminanceSlider);
            ui->LuminanceSlider->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_AlphaSet:
        {
            bool checked = ui->AlphaCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->AlphaCheckBox);
            const QSignalBlocker blocker1(ui->AlphaSpinBox);
            const QSignalBlocker blocker2(ui->AlphaSlider);
            ui->AlphaCheckBox->setChecked(checked);
            ui->AlphaSpinBox->setEnabled(checked);
            ui->AlphaSlider->setEnabled(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_Alpha:
        {
            int value = ui->AlphaSpinBox->value();

            const QSignalBlocker blocker0(ui->AlphaSpinBox);
            ui->AlphaSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->AlphaSlider);
            ui->AlphaSlider->setValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadowSet:
        {
            bool checked = ui->ShadowCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShadowCheckBox);
            const QSignalBlocker blocker1(ui->ShadowSpinBox);
            const QSignalBlocker blocker2(ui->ShadowSlider);
            ui->ShadowCheckBox->setChecked(checked);
            ui->ShadowSpinBox->setEnabled(checked);
            ui->ShadowSlider->setEnabled(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_Shadow:
        {
            double value = ui->ShadowSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadowSpinBox);
            ui->ShadowSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ShadowSlider);
            ui->ShadowSlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_ExtractBoundary:
        {
            double value = ui->ExtractBoundarySpinBox->value();

            const QSignalBlocker blocker0(ui->ExtractBoundarySpinBox);
            ui->ExtractBoundarySpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ExtractBoundarySlider);
            ui->ExtractBoundarySlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadingSet:
        {
            bool checked = ui->ShadowCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShadowCheckBox);
            const QSignalBlocker blocker1(ui->ShadingIntensitySpinBox);
            const QSignalBlocker blocker2(ui->ShadingIntensitySlider);
            const QSignalBlocker blocker3(ui->ShadingShinninessSpinBox);
            const QSignalBlocker blocker4(ui->ShadingShinninessSlider);
            const QSignalBlocker blocker5(ui->ShadingThresholdMinSpinBox);
            const QSignalBlocker blocker6(ui->ShadingThresholdMaxSpinBox);
            const QSignalBlocker blocker7(ui->ShadingThresholdSlider);

            ui->ShadowCheckBox->setChecked(checked);
            ui->ShadingIntensityLabel->setEnabled(checked);
            ui->ShadingIntensitySpinBox->setEnabled(checked);
            ui->ShadingIntensitySlider->setEnabled(checked);
            ui->ShadingShinninessLabel->setEnabled(checked);
            ui->ShadingShinninessSpinBox->setEnabled(checked);
            ui->ShadingShinninessSlider->setEnabled(checked);

            ui->ShadingThresholdLabel->setEnabled(checked);
            ui->ShadingThresholdMinSpinBox->setEnabled(checked);
            ui->ShadingThresholdMaxSpinBox->setEnabled(checked);
            ui->ShadingThresholdSlider->setEnabled(checked);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadingIntensity:
        {
            double value = ui->ShadingIntensitySpinBox->value();

            const QSignalBlocker blocker0(ui->ShadingIntensitySpinBox);
            ui->ShadingIntensitySpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ShadingIntensitySlider);
            ui->ShadingIntensitySlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadingShinniness:
        {
            double value = ui->ShadingShinninessSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadingShinninessSpinBox);
            ui->ShadingShinninessSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ShadingShinninessSlider);
            ui->ShadingShinninessSlider->setValue(value*100.0);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadingThresholdMin:
        {
            int value = ui->ShadingThresholdMinSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadingThresholdMinSpinBox);
            const QSignalBlocker blocker1(ui->ShadingThresholdSlider);
            ui->ShadingThresholdMinSpinBox->setValue(value);
            ui->ShadingThresholdSlider->setLowerValue(value);
        }
            break;
        case VolumePropertiesAttributes::ID_ShadingThresholdMax:
        {
            int value = ui->ShadingThresholdMaxSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadingThresholdMaxSpinBox);
            const QSignalBlocker blocker1(ui->ShadingThresholdSlider);
            ui->ShadingThresholdMaxSpinBox->setValue(value);
            ui->ShadingThresholdSlider->setUpperValue(value);
        }
            break;
        }
    }
}

// Dialog
void QvisVolumePropertiesDialog::ResetAllButtonClicked()
{

}

void QvisVolumePropertiesDialog::MakeDefaultButtonClicked()
{

}

// Rendering
void QvisVolumePropertiesDialog::XVoxelValueChanged(double value)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::YVoxelValueChanged(double value)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::ZVoxelValueChanged(double value)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::RenderingMethodCurrentIndexChanged(int index)
{
    // Update attributes.

    RenderMethodEnable();
}

void QvisVolumePropertiesDialog::ColorSourceCurrentIndexChanged(int index)
{
    // Update attributes.

    ColorSourceEnable();
}

void QvisVolumePropertiesDialog::PrimaryColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(255,255,255);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->PrimaryColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
     // Update attributes.
    }

    this->raise();
}

void QvisVolumePropertiesDialog::SecondaryColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(255,255,255);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->SecondaryColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
     // Update attributes.
    }

    this->raise();
}

void QvisVolumePropertiesDialog::ColorTableCurrentIndexChanged(int index)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::ColorDataSourceCurrentIndexChanged(int index)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::DataRangeSliderLowerChanged(int value)
{
    this->DataMinimumValueChanged(value);
}

void QvisVolumePropertiesDialog::DataMinimumValueChanged(int value)
{
    const QSignalBlocker blocker0(ui->DataMinimumSpinBox);
    const QSignalBlocker blocker1(ui->DataRangeSlider);

    if(value > ui->DataMaximumSpinBox->value())
        value = ui->DataMaximumSpinBox->value();

    ui->DataMinimumSpinBox->setValue(value);
    ui->DataMinimumSpinBox->setFocus();
    ui->DataRangeSlider->setLowerValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::DataRangeSliderUpperChanged(int value)
{
    this->DataMaximumValueChanged(value);
}

void QvisVolumePropertiesDialog::DataMaximumValueChanged(int value)
{
    const QSignalBlocker blocker0(ui->DataMaximumSpinBox);
    const QSignalBlocker blocker1(ui->DataRangeSlider);

    if(value < ui->DataMinimumSpinBox->value())
        value = ui->DataMinimumSpinBox->value();

    ui->DataMaximumSpinBox->setValue(value);
    ui->DataMaximumSpinBox->setFocus();
    ui->DataRangeSlider->setUpperValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::SampleRateSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SampleRateSpinBox);
    ui->SampleRateSpinBox->setValue(double(value)/100.0);
    ui->SampleRateSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::SampleRateValueChanged(double value)
{
    const QSignalBlocker blocker(ui->SampleRateSlider);
    ui->SampleRateSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::HighTransparancyModeToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::ShowComponentsToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::SmoothFinalImageToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::InvertDataValuesToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::UseInterpolationToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::EnableDepthModeToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::ShowLegendToggled(bool checked)
{
    // Update attributes.
}

void QvisVolumePropertiesDialog::SyncAttributesToggled(bool checked)
{
    // Update attributes.
}

// Transfer
void QvisVolumePropertiesDialog::GammalSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->GammaSpinBox);
    ui->GammaSpinBox->setValue(double(value)/100.0);
    ui->GammaSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::GammaValueChanged(double value)
{
    const QSignalBlocker blocker(ui->GammaSlider);
    ui->GammaSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::SaturationSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SaturationSpinBox);
    ui->SaturationSpinBox->setValue(value);
    ui->SaturationSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::SaturationValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SaturationSlider);
    ui->SaturationSlider->setValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::LuminanceSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->LuminanceSpinBox);
    ui->LuminanceSpinBox->setValue(value);
    ui->LuminanceSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::LuminanceValueChanged(int value)
{
    const QSignalBlocker blocker(ui->LuminanceSlider);
    ui->LuminanceSlider->setValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::AlphaToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->AlphaSpinBox);
    const QSignalBlocker blocker1(ui->AlphaSlider);
    ui->AlphaSpinBox->setEnabled(checked);
    ui->AlphaSlider->setEnabled(checked);

    // Update attributes.
}

void QvisVolumePropertiesDialog::AlphaSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->AlphaSpinBox);
    ui->AlphaSpinBox->setValue(value);
    ui->AlphaSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::AlphaValueChanged(int value)
{
    const QSignalBlocker blocker(ui->AlphaSlider);
    ui->AlphaSlider->setValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadowToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->ShadowSpinBox);
    const QSignalBlocker blocker1(ui->ShadowSlider);
    ui->ShadowSpinBox->setEnabled(checked);
    ui->ShadowSlider->setEnabled(checked);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadowSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadowSpinBox);
    ui->ShadowSpinBox->setValue(double(value)/100.0);
    ui->ShadowSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadowValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ShadowSlider);
    ui->ShadowSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ExtractBoundarySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ExtractBoundarySpinBox);
    ui->ExtractBoundarySpinBox->setValue(double(value)/100.0);
    ui->ExtractBoundarySpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::ExtractBoundaryValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ExtractBoundarySlider);
    ui->ExtractBoundarySlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->ShadingIntensitySpinBox);
    const QSignalBlocker blocker1(ui->ShadingIntensitySlider);
    const QSignalBlocker blocker2(ui->ShadingShinninessSpinBox);
    const QSignalBlocker blocker3(ui->ShadingShinninessSlider);
    const QSignalBlocker blocker4(ui->ShadingThresholdMinSpinBox);
    const QSignalBlocker blocker5(ui->ShadingThresholdMaxSpinBox);
    const QSignalBlocker blocker6(ui->ShadingThresholdSlider);

    // Not really needed as the group box does the enabling/disabling
    ui->ShadingIntensityLabel->setEnabled(checked);
    ui->ShadingIntensitySpinBox->setEnabled(checked);
    ui->ShadingIntensitySlider->setEnabled(checked);
    ui->ShadingShinninessLabel->setEnabled(checked);
    ui->ShadingShinninessSpinBox->setEnabled(checked);
    ui->ShadingShinninessSlider->setEnabled(checked);

    // Not really needed as the group box does the enabling/disabling
    ui->ShadingThresholdLabel->setEnabled(checked);
    ui->ShadingThresholdMinSpinBox->setEnabled(checked);
    ui->ShadingThresholdMaxSpinBox->setEnabled(checked);
    ui->ShadingThresholdSlider->setEnabled(checked);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingIntensitySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadingIntensitySpinBox);
    ui->ShadingIntensitySpinBox->setValue(double(value)/100.0);
    ui->ShadingIntensitySpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingIntensityValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ShadingIntensitySlider);
    ui->ShadingIntensitySlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingShinninessSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadingShinninessSpinBox);
    ui->ShadingShinninessSpinBox->setValue(double(value)/100.0);
    ui->ShadingShinninessSpinBox->setFocus();

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingShinninessValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ShadingShinninessSlider);
    ui->ShadingShinninessSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingThresholdSliderLowerChanged(int aLowerValue)
{
    this->ShadingThresholdMinValueChanged(aLowerValue);
}

void QvisVolumePropertiesDialog::ShadingThresholdMinValueChanged(int value)
{
    const QSignalBlocker blocker0(ui->ShadingThresholdMinSpinBox);
    const QSignalBlocker blocker1(ui->ShadingThresholdSlider);

    if(value > ui->ShadingThresholdMaxSpinBox->value())
        value = ui->ShadingThresholdMaxSpinBox->value();

    ui->ShadingThresholdMinSpinBox->setValue(value);
    ui->ShadingThresholdMinSpinBox->setFocus();
    ui->ShadingThresholdSlider->setLowerValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::ShadingThresholdSliderUpperChanged(int aUpperValue)
{
    this->ShadingThresholdMaxValueChanged(aUpperValue);
}

void QvisVolumePropertiesDialog::ShadingThresholdMaxValueChanged(int value)
{
    const QSignalBlocker blocker0(ui->ShadingThresholdMaxSpinBox);
    const QSignalBlocker blocker1(ui->ShadingThresholdSlider);

    if(value < ui->ShadingThresholdMinSpinBox->value())
        value = ui->ShadingThresholdMinSpinBox->value();

    ui->ShadingThresholdMaxSpinBox->setValue(value);
    ui->ShadingThresholdMaxSpinBox->setFocus();
    ui->ShadingThresholdSlider->setUpperValue(value);

    // Update attributes.
}

void QvisVolumePropertiesDialog::RenderMethodEnable()
{
    int index = ui->RenderingMethodComboBox->currentIndex();

    VolumePropertiesAttributes::RenderingMethod method =
        VolumePropertiesAttributes::RenderingMethod(index);

    const QSignalBlocker blocker1(ui->GammaSlider);
    const QSignalBlocker blocker2(ui->GammaSpinBox);
    const QSignalBlocker blocker3(ui->SaturationSlider);
    const QSignalBlocker blocker4(ui->SaturationSpinBox);
    const QSignalBlocker blocker5(ui->LuminanceSlider);
    const QSignalBlocker blocker6(ui->LuminanceSpinBox);
    const QSignalBlocker blocker7(ui->AlphaCheckBox);
    const QSignalBlocker blocker8(ui->AlphaSlider);
    const QSignalBlocker blocker9(ui->AlphaSpinBox);
    const QSignalBlocker blocker10(ui->ShadowCheckBox);
    const QSignalBlocker blocker11(ui->ShadowSlider);
    const QSignalBlocker blocker12(ui->ShadowSpinBox);
    const QSignalBlocker blocker13(ui->ExtractBoundarySlider);
    const QSignalBlocker blocker14(ui->ExtractBoundarySpinBox);
    const QSignalBlocker blocker15(ui->ShadingGroupBox);
    const QSignalBlocker blocker16(ui->ShadingIntensitySlider);
    const QSignalBlocker blocker17(ui->ShadingIntensitySpinBox);
    const QSignalBlocker blocker18(ui->ShadingShinninessSlider);
    const QSignalBlocker blocker19(ui->ShadingShinninessSpinBox);
    const QSignalBlocker blocker20(ui->ShadingThresholdMinSpinBox);
    const QSignalBlocker blocker21(ui->ShadingThresholdMaxSpinBox);
    const QSignalBlocker blocker22(ui->ShadingThresholdSlider);

    const QSignalBlocker blocker23(ui->SampleRateSpinBox);
    const QSignalBlocker blocker24(ui->SampleRateSlider);

    bool volume = (method == VolumePropertiesAttributes::Volume);
    bool mip = (method == VolumePropertiesAttributes::MIP);
    bool shading = ui->ShadingGroupBox->isChecked();

    ui->GammaLabel->setEnabled(volume | mip);
    ui->GammaSlider->setEnabled(volume | mip);
    ui->GammaSpinBox->setEnabled(volume | mip);
    ui->SaturationLabel->setEnabled(volume | mip);
    ui->SaturationSlider->setEnabled(volume | mip);
    ui->SaturationSpinBox->setEnabled(volume | mip);
    ui->LuminanceLabel->setEnabled(volume);
    ui->LuminanceSlider->setEnabled(volume);
    ui->LuminanceSpinBox->setEnabled(volume);
    ui->AlphaCheckBox->setEnabled(volume);
    ui->AlphaSlider->setEnabled(volume);
    ui->AlphaSpinBox->setEnabled(volume);
    ui->ShadowCheckBox->setEnabled(volume);
    ui->ShadowSlider->setEnabled(volume);
    ui->ShadowSpinBox->setEnabled(volume);
    ui->ExtractBoundaryLabel->setEnabled(volume);
    ui->ExtractBoundarySlider->setEnabled(volume);
    ui->ExtractBoundarySpinBox->setEnabled(volume);

    ui->ShadingGroupBox->setEnabled(volume | mip);
    ui->ShadingIntensityLabel->setEnabled(volume | mip | shading);
    ui->ShadingIntensitySlider->setEnabled(volume | mip | shading);
    ui->ShadingIntensitySpinBox->setEnabled(volume | mip | shading);
    ui->ShadingShinninessLabel->setEnabled(volume | mip | shading);
    ui->ShadingShinninessSlider->setEnabled(volume | mip | shading);
    ui->ShadingShinninessSpinBox->setEnabled(volume | mip | shading);
    ui->ShadingThresholdLabel->setEnabled(volume | mip | shading);
    ui->ShadingThresholdMinSpinBox->setEnabled(volume | mip | shading);
    ui->ShadingThresholdMaxSpinBox->setEnabled(volume | mip | shading);
    ui->ShadingThresholdSlider->setEnabled(volume | mip | shading);

    ui->SampleRateSpinBox->setEnabled(volume | mip);
    ui->SampleRateSlider->setEnabled(volume | mip);
}

void QvisVolumePropertiesDialog::ColorSourceEnable()
{
    int index = ui->ColorSourceComboBox->currentIndex();

    VolumePropertiesAttributes::ColorSource source =
        VolumePropertiesAttributes::ColorSource(index);

    bool solidColor = (source == VolumePropertiesAttributes::Solid);

    const QSignalBlocker blocker1(ui->ColorGroupBox);
    const QSignalBlocker blocker2(ui->PrimaryColorButton);
    const QSignalBlocker blocker3(ui->SecondaryColorButton);
    ui->ColorGroupBox->setEnabled(solidColor);
    ui->PrimaryColorLabel->setEnabled(solidColor);
    ui->PrimaryColorButton->setEnabled(solidColor);
    ui->SecondaryColorLabel->setEnabled(solidColor);
    ui->SecondaryColorButton->setEnabled(solidColor);

    bool colorMap = (source == VolumePropertiesAttributes::ColorMap);

    const QSignalBlocker blocker4(ui->ColorTableComboBox);
    const QSignalBlocker blocker5(ui->ColorDataSourceComboBox);
    ui->ColorMappingGroupBox->setEnabled(colorMap);
    ui->ColorTableLabel->setEnabled(colorMap);
    ui->ColorTableComboBox->setEnabled(colorMap);
    ui->ColorDataSourceLabel->setEnabled(colorMap);
    ui->ColorDataSourceComboBox->setEnabled(colorMap);
}
