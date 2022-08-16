#ifndef QVISVIEWPROPERTIESDIALOG_H
#define QVISVIEWPROPERTIESDIALOG_H

#include "QvisDialogBase.h"

class ViewPropertiesAttributes
{
public:
    enum {
        // View
        ID_Scale = 0,
        ID_ScaleModeType,
        ID_AxisAlignmentType,
        ID_RotateX,
        ID_RotateY,
        ID_RotateZ,
        ID_RotateModeType,
        ID_RotateOffsetType,
        ID_RotateAnchorType,
        ID_RotateAnchor,
        ID_RotateLinkViewsSet,
        ID_PerspectiveSet,
        ID_AngleOfView,
        ID_CameraModeType,
        // Annotation
        ID_FontType,
        ID_FontSize,
        ID_FontColorType,
        ID_ShowDatasetNamesSet,
        ID_ShowColormapSampleSet,
        ID_ShowAxisSet,
        ID_ShowMousePositionSet,
        ID_LineWidth,
        ID_ScaleBarSet,
        ID_ScaleBarLength,
        ID_ScaleBarUnitsSet,
        ID_ScaleBarUnitsType,
        ID_ForegroundColor,
        ID_BackgroundColor,
        ID_GradientBackgroundSet,
        // Rendering
        ID_RenderingDepthSet,
        ID_RenderingDepthInterval,
        ID_RenderViewByType,
        ID_MicroBlendingSet,
        ID_MeshTransParencyQuality,
        ID_ShaddowsSet,
        ID_ShadowAngle,
        ID_StereoSteamVRSet,
        ID_StereoEyeDistance,
        ID_LAST_VARIABLE
    };

    enum ScaleMode {
        PixelBased,
        DataBased,
        ViewBased,
    };

    enum AxisAlignment {
            XPositive,
            YPositive,
            ZPositive,
            XNegative,
            YNegative,
            ZNegative,
    };

    enum RotateMode {
        FollowSlider,
        Spin,
    };

    enum RotateOffset {
        OneDegree,
        TenDegrees,
        FifteenDegrees,
        ThirtyDegrees,
        FortyFiveDegrees,
    };

    enum RotateAnchor {
        AxisOrigin,
        ImageCentroid,
    };

    enum CameraMode {
        Normal,
        FreeFly,
    };

    enum Font {
        FreeMono,
        FreeMonoBold,
        FreeMonoOblique,
        FreeMonoBoldOblique,
        FreeSans,
        FreeSansBold,
        FreeSansOblique,
        FreeSansBoldOblique,
        FreeSerif,
        FreeSerifBold,
        FreeSerifOblique,
        FreeSerifBoldOblique,
    };

    enum FontColor {
        InvertedBackground,
        Background,
        SecondaryColor,
    };

    enum ScaleBarUnits {
        nanometers,
        micrometers,
        millimeters,
    };

    enum RenderViewBy {
        Layers,
        Depth,
        CopmpositeOfColors,
    };

    bool isSelected(int i) { return false; };
    int numAttributes() { return ID_LAST_VARIABLE; };
};

namespace Ui {
class QvisViewPropertiesDialog;
}

class QvisViewPropertiesDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisViewPropertiesDialog(QWidget *parent = nullptr);
    ~QvisViewPropertiesDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ResetAllButtonClicked();
    void MakeDefaultButtonClicked();
    // View
    void ScaleSliderChanged(int value);
    void ScaleValueChanged(int value);
    void ScaleModeIndexChanged(int index);
    void ResetScaleButtonClicked();
    void AxisAlignmentIndexChanged(int index);
    void RotateXSliderChanged(int value);
    void RotateXValueChanged(int value);
    void RotateYSliderChanged(int value);
    void RotateYValueChanged(int value);
    void RotateZSliderChanged(int value);
    void RotateZValueChanged(int value);
    void RotateModeIndexChanged(int index);
    void RotateOffsetIndexChanged(int index);
    void RotateAnchorIndexChanged(int index);
    void RotateAnchorSliderChanged(int value);
    void RotateAnchorValueChanged(int value);
    void RotateLinkViewsToggled(bool checked);
    void ResetRotationButtonClicked();
    void PerspectiveToggled(bool checked);
    void AngleOfViewSliderChanged(int value);
    void AngleOfViewValueChanged(int value);
    void CameraModeIndexChanged(int index);
    // Annotation
    void FontIndexChanged(int index);
    void FontSizeValueChanged(int value);
    void FontColorIndexChanged(int index);
    void ShowDatasetNamesToggled(bool checked);
    void ShowColormapSampleToggled(bool checked);
    void ShowAxisToggled(bool checked);
    void ShowMousePositionToggled(bool checked);
    void LineWidthSliderChanged(int value);
    void LineWidthValueChanged(int value);
    void ScaleBarToggled(bool checked);
    void ScaleBarUnitsToggled(bool checked);
    void ScaleBarLengthValueChanged(int value);
    void ScaleBarUnitsIndexChanged(int index);
    void ForegroundColorButtonClicked();
    void BackgroundColorButtonClicked();
    void GradientBackgroundToggled(bool checked);
    // Rendering
    void RenderingDepthToggled(bool checked);
    void RenderingDepthIntervalSliderChanged(int value);
    void RenderingDepthIntervalValueChanged(double value);
    void RenderViewByIndexChanged(int index);
    void MicroBlendingToggled(bool checked);
    void MeshTransParencyQualitySliderChanged(int value);
    void MeshTransParencyQualityValueChanged(int value);
    void ShaddowsToggled(bool checked);
    void ShadowAngleSliderChanged(int value);
    void ShadowAngleValueChanged(int value);
    void StereoSteamVRToggled(bool checked);
    void StereoEyeDistanceSliderChanged(int value);
    void StereoEyeDistanceValueChanged(int value);

private:
    Ui::QvisViewPropertiesDialog *ui{nullptr};

    // Variables
    ViewPropertiesAttributes* mAttributes{nullptr};
};

#endif // QVISVIEWPROPERTIESDIALOG_H
