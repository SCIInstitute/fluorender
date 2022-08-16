#ifndef QVISMESHPROPERTIESDIALOG_H
#define QVISMESHPROPERTIESDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisMeshPropertiesDialog;
}

class MeshPropertiesAttributes
{
public:

    enum {
        ID_DifuseColor = 0,
        ID_SpecularColor,
        // Scaling/Size
        ID_Scaling,
        ID_SizeLimitSet,
        ID_SizeLimit,
        // Rendering
        ID_Shininess,
        ID_Transparency,
        ID_ShadowSet,
        ID_Shadow,
        ID_LightingSet,
        ID_LAST_VARIABLE
    };

    bool isSelected(int i) { return false; };
    int numAttributes() { return ID_LAST_VARIABLE; };
};


class QvisMeshPropertiesDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisMeshPropertiesDialog(QWidget *parent = nullptr);
    ~QvisMeshPropertiesDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    // Color
    void DifuseColorButtonClicked();
    void SpecularColorButtonClicked();
    // Scaling/Size
    void ScalingSliderChanged(int value);
    void ScalingValueChanged(double value);
    void SizeLimitToggled(bool checked);
    void SizeLimitSliderChanged(int value);
    void SizeLimitValueChanged(int value);
    // Rendering
    void ShininessSliderChanged(int value);
    void ShininessValueChanged(int value);
    void TransparencySliderChanged(int value);
    void TransparencyValueChanged(double value);
    void ShadowToggled(bool checked);
    void ShadowSliderChanged(int value);
    void ShadowValueChanged(double value);
    void LightingToggled(bool checked);

private:
    Ui::QvisMeshPropertiesDialog *ui{nullptr};

    // Variables
    MeshPropertiesAttributes* mAttributes{nullptr};
};

#endif // QVISMESHPROPERTIESDIALOG_H
