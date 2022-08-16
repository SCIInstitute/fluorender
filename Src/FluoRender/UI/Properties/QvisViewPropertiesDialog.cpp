#include "QvisViewPropertiesDialog.h"
#include "ui_QvisViewPropertiesDialog.h"

#include <QColorDialog>

QvisViewPropertiesDialog::QvisViewPropertiesDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisViewPropertiesDialog)
{
    ui->setupUi(this);
}

QvisViewPropertiesDialog::~QvisViewPropertiesDialog()
{
    delete ui;
}

void QvisViewPropertiesDialog::updateWindow(bool doAll)
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
        // View
        case ViewPropertiesAttributes::ID_Scale:
        {
            int value = ui->ScaleSpinBox->value();

            const QSignalBlocker blocker0(ui->ScaleSpinBox);
            ui->ScaleSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ScaleSlider);
            ui->ScaleSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ScaleModeType:
        {
            int value = ui->ScaleModeComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->ScaleModeComboBox);
            ui->ScaleModeComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_AxisAlignmentType:
        {
            int value = ui->AxisAlignmentComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->AxisAlignmentComboBox);
            ui->AxisAlignmentComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateX:
        {
            int value = ui->RotateXSpinBox->value();

            const QSignalBlocker blocker0(ui->RotateXSpinBox);
            ui->RotateXSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->RotateXSlider);
            ui->RotateXSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateY:
        {
            int value = ui->RotateYSpinBox->value();

            const QSignalBlocker blocker0(ui->RotateYSpinBox);
            ui->RotateYSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->RotateYSlider);
            ui->RotateYSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateZ:
        {
            int value = ui->RotateZSpinBox->value();

            const QSignalBlocker blocker0(ui->RotateZSpinBox);
            ui->RotateZSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->RotateZSlider);
            ui->RotateZSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateModeType:
        {
            int value = ui->RotateModeComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->RotateModeComboBox);
            ui->RotateModeComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateOffsetType:
        {
            int value = ui->RotateModeComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->RotateModeComboBox);
            ui->RotateModeComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateAnchorType:
        {
            int value = ui->RotateAnchorComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->RotateAnchorComboBox);
            ui->RotateAnchorComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateAnchor:
        {
            int value = ui->RototeCenterAnchorStartSpinBox->value();

            const QSignalBlocker blocker0(ui->RototeCenterAnchorStartSpinBox);
            ui->RototeCenterAnchorStartSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->RotateCenterAnchorStartSlider);
            ui->RotateCenterAnchorStartSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_RotateLinkViewsSet:
        {
            bool checked = ui->RotateLinkViewsCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->RotateLinkViewsCheckBox);
            ui->RotateLinkViewsCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_PerspectiveSet:
        {
            bool checked = ui->PerspectiveGroupBox->isChecked();

            // Not really needed as the group box does the enabling/disabling
            const QSignalBlocker blocker0(ui->PerspectiveGroupBox);
            const QSignalBlocker blocker1(ui->AngleOfViewSpinBox);
            const QSignalBlocker blocker2(ui->AngleOfViewSlider);
            ui->PerspectiveGroupBox->setChecked(checked);
            ui->AngleOfViewLabel->setEnabled(checked);
            ui->AngleOfViewSpinBox->setEnabled(checked);
            ui->AngleOfViewSlider->setEnabled(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_AngleOfView:
        {
            int value = ui->AngleOfViewSpinBox->value();

            const QSignalBlocker blocker0(ui->AngleOfViewSpinBox);
            ui->AngleOfViewSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->AngleOfViewSlider);
            ui->AngleOfViewSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_CameraModeType:
        {
            int value = ui->CameraModeComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->CameraModeComboBox);
            ui->CameraModeComboBox->setCurrentIndex(value);
        }
            break;
        // Annotation
        case ViewPropertiesAttributes::ID_FontType:
        {
            int value = ui->FontNameComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->FontNameComboBox);
            ui->FontNameComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_FontSize:
        {
            int value = ui->FontSizeSpinBox->value();

            const QSignalBlocker blocker0(ui->FontSizeSpinBox);
            ui->FontSizeSpinBox->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_FontColorType:
        {
            int value = ui->FontColorComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->FontColorComboBox);
            ui->FontColorComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ShowDatasetNamesSet:
        {
            bool checked = ui->ShowDatasetNamesCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowDatasetNamesCheckBox);
            ui->ShowDatasetNamesCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_ShowColormapSampleSet:
        {
            bool checked = ui->ShowColormapSampleCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowColormapSampleCheckBox);
            ui->ShowColormapSampleCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_ShowAxisSet:
        {
            bool checked = ui->ShowAxisCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowAxisCheckBox);
            ui->ShowAxisCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_ShowMousePositionSet:
        {
            bool checked = ui->ShowMousePositionCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ShowMousePositionCheckBox);
            ui->ShowMousePositionCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_LineWidth:
        {
            int value = ui->LineWidthSlider->value();

            const QSignalBlocker blocker0(ui->LineWidthSpinBox);
            ui->LineWidthSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->LineWidthSlider);
            ui->LineWidthSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ScaleBarSet:
        {
            bool checked = ui->ScaleBarGroupBox->isChecked();

            // Not really needed as the group box does the enabling/disabling
            const QSignalBlocker blocker0(ui->ScaleBarGroupBox);
            const QSignalBlocker blocker1(ui->ScaleBarLengthSpinBox);
            const QSignalBlocker blocker2(ui->ScaleBarUnitsCheckBox);
            const QSignalBlocker blocker3(ui->ScaleBarUnitsComboBox);
            ui->ScaleBarGroupBox->setChecked(checked);
            ui->ScaleBarLengthLabel->setEnabled(checked);
            ui->ScaleBarLengthSpinBox->setEnabled(checked);
            ui->ScaleBarUnitsCheckBox->setEnabled(checked);
            ui->ScaleBarUnitsComboBox->setEnabled(checked & ui->ScaleBarUnitsCheckBox->isChecked());
        }
            break;
        case ViewPropertiesAttributes::ID_ScaleBarLength:
        {
            int value = ui->ScaleBarLengthSpinBox->value();

            const QSignalBlocker blocker2(ui->ScaleBarLengthSpinBox);
            ui->ScaleBarLengthSpinBox->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ScaleBarUnitsSet:
        {
            bool checked = ui->ScaleBarUnitsCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->ScaleBarUnitsCheckBox);
            const QSignalBlocker blocker1(ui->ScaleBarUnitsComboBox);
            ui->ScaleBarUnitsCheckBox->setChecked(checked);
            ui->ScaleBarUnitsComboBox->setEnabled(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_ScaleBarUnitsType:
        {
            int value = ui->ScaleBarUnitsComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->ScaleBarUnitsComboBox);
            ui->ScaleBarUnitsComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ForegroundColor:
        {
            QColor color(255,255,255);
            ui->ForegroundColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
            break;
        case ViewPropertiesAttributes::ID_BackgroundColor:
        {
            QColor color(255,255,255);
            ui->BackgroundColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
            break;
        case ViewPropertiesAttributes::ID_GradientBackgroundSet:
        {
            bool checked = ui->GradientBackgroundCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->GradientBackgroundCheckBox);
            ui->GradientBackgroundCheckBox->setChecked(checked);
        }
            break;
        // Rendering
        case ViewPropertiesAttributes::ID_RenderingDepthSet:
        {
            bool checked = ui->RenderingDepthGroupBox->isChecked();

            const QSignalBlocker blocker0(ui->RenderingDepthGroupBox);
            const QSignalBlocker blocker1(ui->RenderingDepthIntervalSlider);
            const QSignalBlocker blocker2(ui->RenderingDepthIntervalSpinBox);
            ui->RenderingDepthGroupBox->setChecked(checked);
            ui->RenderingDepthIntervalLabel->setEnabled(checked);
            ui->RenderingDepthIntervalSlider->setEnabled(checked);
            ui->RenderingDepthIntervalSpinBox->setEnabled(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_RenderingDepthInterval:
        {
            double value = ui->RenderingDepthIntervalSpinBox->value();

            const QSignalBlocker blocker0(ui->RenderingDepthIntervalSpinBox);
            ui->RenderingDepthIntervalSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->RenderingDepthIntervalSlider);
            ui->RenderingDepthIntervalSlider->setValue(value*100.0);
        }
            break;
        case ViewPropertiesAttributes::ID_RenderViewByType:
        {
            int value = ui->RenderViewByComboBox->currentIndex();

            const QSignalBlocker blocker0(ui->RenderViewByComboBox);
            ui->RenderViewByComboBox->setCurrentIndex(value);
        }
            break;
        case ViewPropertiesAttributes::ID_MicroBlendingSet:
        {
            bool checked = ui->MicroBlendingCheckBox->isChecked();

            const QSignalBlocker blocker0(ui->MicroBlendingCheckBox);
            ui->MicroBlendingCheckBox->setChecked(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_MeshTransParencyQuality:
        {
            int value = ui->MeshTransparencyQualitySpinBox->value();

            const QSignalBlocker blocker0(ui->MeshTransparencyQualitySpinBox);
            ui->MeshTransparencyQualitySpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->MeshTransparencyQualitySlider);
            ui->MeshTransparencyQualitySlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_ShaddowsSet:
        {
            bool checked = ui->ShaddowsGroupBox->isChecked();

            const QSignalBlocker blocker0(ui->ShaddowsGroupBox);
            const QSignalBlocker blocker1(ui->ShadowAngleSpinBox);
            const QSignalBlocker blocker2(ui->ShadowAngleSlider);
            ui->ShaddowsGroupBox->setChecked(checked);
            ui->ShadowAngleLabel->setEnabled(checked);
            ui->ShadowAngleSpinBox->setEnabled(checked);
            ui->ShadowAngleSlider->setEnabled(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_ShadowAngle:
        {
            int value = ui->ShadowAngleSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadowAngleSpinBox);
            ui->ShadowAngleSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->ShadowAngleSlider);
            ui->ShadowAngleSlider->setValue(value);
        }
            break;
        case ViewPropertiesAttributes::ID_StereoSteamVRSet:
        {
            bool checked = ui->StereoSteamVRGroupBox->isChecked();

            const QSignalBlocker blocker0(ui->StereoSteamVRGroupBox);
            const QSignalBlocker blocker1(ui->StereoEyeDistanceSpinBox);
            const QSignalBlocker blocker2(ui->StereoEyeDistanceSlider);
            ui->StereoSteamVRGroupBox->setChecked(checked);
            ui->StereoEyeDistanceLabel->setEnabled(checked);
            ui->StereoEyeDistanceSpinBox->setEnabled(checked);
            ui->StereoEyeDistanceSlider->setEnabled(checked);
        }
            break;
        case ViewPropertiesAttributes::ID_StereoEyeDistance:
        {
            int value = ui->StereoEyeDistanceSpinBox->value();

            const QSignalBlocker blocker0(ui->StereoEyeDistanceSpinBox);
            ui->StereoEyeDistanceSpinBox->setValue(value);
            const QSignalBlocker blocker1(ui->StereoEyeDistanceSlider);
            ui->StereoEyeDistanceSlider->setValue(value);
        }
            break;
        }
    }
}

// Main dialog
void QvisViewPropertiesDialog::ResetAllButtonClicked()
{

}

// View
void QvisViewPropertiesDialog::MakeDefaultButtonClicked()
{

}

void QvisViewPropertiesDialog::ScaleSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ScaleSpinBox);
    ui->ScaleSpinBox->setValue(value);
    ui->ScaleSpinBox->setFocus();
}

void QvisViewPropertiesDialog::ScaleValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ScaleSlider);
    ui->ScaleSlider->setValue(value);
}

void QvisViewPropertiesDialog::ScaleModeIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::ResetScaleButtonClicked()
{

}

void QvisViewPropertiesDialog::AxisAlignmentIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::RotateXSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateXSpinBox);
    ui->RotateXSpinBox->setValue(value);
    ui->RotateXSpinBox->setFocus();
}

void QvisViewPropertiesDialog::RotateXValueChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateXSlider);
    ui->RotateXSlider->setValue(value);
}

void QvisViewPropertiesDialog::RotateYSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateYSpinBox);
    ui->RotateYSpinBox->setValue(value);
    ui->RotateYSpinBox->setFocus();
}

void QvisViewPropertiesDialog::RotateYValueChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateYSlider);
    ui->RotateYSlider->setValue(value);
}

void QvisViewPropertiesDialog::RotateZSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateZSpinBox);
    ui->RotateZSpinBox->setValue(value);
    ui->RotateZSpinBox->setFocus();
}

void QvisViewPropertiesDialog::RotateZValueChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateZSlider);
    ui->RotateZSlider->setValue(value);
}

void QvisViewPropertiesDialog::RotateModeIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::RotateOffsetIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::RotateAnchorIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::RotateAnchorSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->RototeCenterAnchorStartSpinBox);
    ui->RototeCenterAnchorStartSpinBox->setValue(value);
    ui->RototeCenterAnchorStartSpinBox->setFocus();
}

void QvisViewPropertiesDialog::RotateAnchorValueChanged(int value)
{
    const QSignalBlocker blocker(ui->RotateCenterAnchorStartSlider);
    ui->RotateCenterAnchorStartSlider->setValue(value);
}

void QvisViewPropertiesDialog::RotateLinkViewsToggled(bool checked)
{

}

void QvisViewPropertiesDialog::ResetRotationButtonClicked()
{

}

void QvisViewPropertiesDialog::PerspectiveToggled(bool checked)
{
    // Not really needed as the group box does the enabling/disabling
    const QSignalBlocker blocker0(ui->AngleOfViewSpinBox);
    const QSignalBlocker blocker1(ui->AngleOfViewSlider);
    ui->AngleOfViewLabel->setEnabled(checked);
    ui->AngleOfViewSpinBox->setEnabled(checked);
    ui->AngleOfViewSlider->setEnabled(checked);
}

void QvisViewPropertiesDialog::AngleOfViewSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->AngleOfViewSpinBox);
    ui->AngleOfViewSpinBox->setValue(value);
    ui->AngleOfViewSpinBox->setFocus();
}

void QvisViewPropertiesDialog::AngleOfViewValueChanged(int value)
{
    const QSignalBlocker blocker(ui->AngleOfViewSlider);
    ui->AngleOfViewSlider->setValue(value);
}

void QvisViewPropertiesDialog::CameraModeIndexChanged(int index)
{

}

// Annotation
void QvisViewPropertiesDialog::FontIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::FontSizeValueChanged(int value)
{

}

void QvisViewPropertiesDialog::FontColorIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::ShowDatasetNamesToggled(bool checked)
{

}

void QvisViewPropertiesDialog::ShowColormapSampleToggled(bool checked)
{

}

void QvisViewPropertiesDialog::ShowAxisToggled(bool checked)
{

}

void QvisViewPropertiesDialog::ShowMousePositionToggled(bool checked)
{

}

void QvisViewPropertiesDialog::LineWidthSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->LineWidthSpinBox);
    ui->LineWidthSpinBox->setValue(value);
    ui->LineWidthSpinBox->setFocus();
}

void QvisViewPropertiesDialog::LineWidthValueChanged(int value)
{
    const QSignalBlocker blocker(ui->LineWidthSlider);
    ui->LineWidthSlider->setValue(value);
}

void QvisViewPropertiesDialog::ScaleBarToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->ScaleBarLengthSpinBox);
    const QSignalBlocker blocker1(ui->ScaleBarUnitsCheckBox);
    const QSignalBlocker blocker2(ui->ScaleBarUnitsComboBox);
    ui->ScaleBarLengthLabel->setEnabled(checked);
    ui->ScaleBarLengthSpinBox->setEnabled(checked);
    ui->ScaleBarUnitsCheckBox->setEnabled(checked);
    ui->ScaleBarUnitsComboBox->setEnabled(checked & ui->ScaleBarUnitsCheckBox->isChecked());
}

void QvisViewPropertiesDialog::ScaleBarUnitsToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->ScaleBarUnitsComboBox);
    ui->ScaleBarUnitsComboBox->setEnabled(checked);
}

void QvisViewPropertiesDialog::ScaleBarLengthValueChanged(int value)
{

}

void QvisViewPropertiesDialog::ScaleBarUnitsIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::ForegroundColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(255,255,255);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->ForegroundColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
     // Update attributes.
    }

    this->raise();
}

void QvisViewPropertiesDialog::BackgroundColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(0,0,0);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->BackgroundColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        // Update attributes.
    }

    this->raise();
}

void QvisViewPropertiesDialog::GradientBackgroundToggled(bool checked)
{

}

// Rendering
void QvisViewPropertiesDialog::RenderingDepthToggled(bool checked)
{
    // Not really needed as the group box does the enabling/disabling
    const QSignalBlocker blocker0(ui->RenderingDepthIntervalSlider);
    const QSignalBlocker blocker1(ui->RenderingDepthIntervalSpinBox);
    ui->RenderingDepthIntervalLabel->setEnabled(checked);
    ui->RenderingDepthIntervalSlider->setEnabled(checked);
    ui->RenderingDepthIntervalSpinBox->setEnabled(checked);
}

void QvisViewPropertiesDialog::RenderingDepthIntervalSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->RenderingDepthIntervalSpinBox);
    ui->RenderingDepthIntervalSpinBox->setValue(double(value)/100.0);
    ui->RenderingDepthIntervalSpinBox->setFocus();
}

void QvisViewPropertiesDialog::RenderingDepthIntervalValueChanged(double value)
{
    const QSignalBlocker blocker(ui->RenderingDepthIntervalSlider);
    ui->RenderingDepthIntervalSlider->setValue(value*100.0);
}

void QvisViewPropertiesDialog::RenderViewByIndexChanged(int index)
{

}

void QvisViewPropertiesDialog::MicroBlendingToggled(bool checked)
{

}

void QvisViewPropertiesDialog::MeshTransParencyQualitySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->MeshTransparencyQualitySpinBox);
    ui->MeshTransparencyQualitySpinBox->setValue(value);
    ui->MeshTransparencyQualitySpinBox->setFocus();
}

void QvisViewPropertiesDialog::MeshTransParencyQualityValueChanged(int value)
{
    const QSignalBlocker blocker(ui->MeshTransparencyQualitySlider);
    ui->MeshTransparencyQualitySlider->setValue(value);
}

void QvisViewPropertiesDialog::ShaddowsToggled(bool checked)
{
    // Not really needed as the group box does the enabling/disabling
    const QSignalBlocker blocker0(ui->ShadowAngleSpinBox);
    const QSignalBlocker blocker1(ui->ShadowAngleSlider);
    ui->ShadowAngleLabel->setEnabled(checked);
    ui->ShadowAngleSpinBox->setEnabled(checked);
    ui->ShadowAngleSlider->setEnabled(checked);
}

void QvisViewPropertiesDialog::ShadowAngleSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadowAngleSpinBox);
    ui->ShadowAngleSpinBox->setValue(value);
    ui->ShadowAngleSpinBox->setFocus();
}

void QvisViewPropertiesDialog::ShadowAngleValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadowAngleSlider);
    ui->ShadowAngleSlider->setValue(value);
}

void QvisViewPropertiesDialog::StereoSteamVRToggled(bool checked)
{
    // Not really needed as the group box does the enabling/disabling
    const QSignalBlocker blocker0(ui->StereoEyeDistanceSpinBox);
    const QSignalBlocker blocker1(ui->StereoEyeDistanceSlider);
    ui->StereoEyeDistanceLabel->setEnabled(checked);
    ui->StereoEyeDistanceSpinBox->setEnabled(checked);
    ui->StereoEyeDistanceSlider->setEnabled(checked);
}

void QvisViewPropertiesDialog::StereoEyeDistanceSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->StereoEyeDistanceSpinBox);
    ui->StereoEyeDistanceSpinBox->setValue(value);
    ui->StereoEyeDistanceSpinBox->setFocus();
}

void QvisViewPropertiesDialog::StereoEyeDistanceValueChanged(int value)
{
    const QSignalBlocker blocker(ui->StereoEyeDistanceSlider);
    ui->StereoEyeDistanceSlider->setValue(value);
}

