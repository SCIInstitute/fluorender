#include "QvisMeshPropertiesDialog.h"
#include "ui_QvisMeshPropertiesDialog.h"

#include <QColorDialog>

QvisMeshPropertiesDialog::QvisMeshPropertiesDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisMeshPropertiesDialog)
{
    ui->setupUi(this);
}

QvisMeshPropertiesDialog::~QvisMeshPropertiesDialog()
{
    delete ui;
}

void QvisMeshPropertiesDialog::updateWindow(bool doAll)
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
        // Color
        case MeshPropertiesAttributes::ID_DifuseColor:
        {
            QColor color(255,255,255);
            ui->DifuseColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
        break;

        case MeshPropertiesAttributes::ID_SpecularColor:
        {
            QColor color(255,255,255);
            ui->SpecularColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }

        // Scaling/Size
        case MeshPropertiesAttributes::ID_Scaling:
        {
            int value = ui->ScalingSpinBox->value();

            const QSignalBlocker blocker0(ui->ScalingSpinBox);
            ui->ScalingSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->ScalingSlider);
            ui->ScalingSlider->setValue(value*100.0);
        }
        break;

        case MeshPropertiesAttributes::ID_SizeLimitSet:
        {
            bool checked = ui->SizeLimitCheckBox->checkState();

            const QSignalBlocker blocker0(ui->SizeLimitSpinBox);
            const QSignalBlocker blocker1(ui->SizeLimitSlider);
            ui->SizeLimitSpinBox->setEnabled(checked);
            ui->SizeLimitSlider->setEnabled(checked);
        }
        break;

        case MeshPropertiesAttributes::ID_SizeLimit:
        {
            int value = ui->SizeLimitSpinBox->value();

            const QSignalBlocker blocker0(ui->SizeLimitSpinBox);
            ui->SizeLimitSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->SizeLimitSlider);
            ui->SizeLimitSlider->setValue(value);
        }
        break;

        // Rendering
        case MeshPropertiesAttributes::ID_Shininess:
        {
            int value = ui->ShininessSpinBox->value();

            const QSignalBlocker blocker0(ui->ShininessSpinBox);
            ui->ShininessSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->ShininessSlider);
            ui->ShininessSlider->setValue(value);
        }
        break;

        case MeshPropertiesAttributes::ID_Transparency:
        {
            double value = ui->ShininessSpinBox->value();

            const QSignalBlocker blocker0(ui->TransparencySpinBox);
            ui->TransparencySpinBox->setValue(value);

            const QSignalBlocker blockerq(ui->TransparencySlider);
            ui->TransparencySlider->setValue(value*100.0);
        }
        break;

        case MeshPropertiesAttributes::ID_ShadowSet:
        {
            bool checked = ui->ShadowCheckBox->checkState();

            const QSignalBlocker blocker0(ui->ShadowSpinBox);
            const QSignalBlocker blocker1(ui->ShadowSlider);
            ui->ShadowSpinBox->setEnabled(checked);
            ui->ShadowSlider->setEnabled(checked);
        }
        break;

        case MeshPropertiesAttributes::ID_Shadow:
        {
            double value = ui->ShadowSpinBox->value();

            const QSignalBlocker blocker0(ui->ShadowSpinBox);
            ui->ShadowSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->ShadowSlider);
            ui->ShadowSlider->setValue(value*100.0);
        }
        break;

        case MeshPropertiesAttributes::ID_LightingSet:
        {
            bool checked = ui->LightingCheckBox->checkState();

            const QSignalBlocker blocker0(ui->ShadowSpinBox);
            ui->LightingCheckBox->setChecked(checked);
        }
        break;
        }
    }
}

// Color
void QvisMeshPropertiesDialog::DifuseColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(255,255,255);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->DifuseColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
     // Update attributes.
    }

    this->raise();
}

void QvisMeshPropertiesDialog::SpecularColorButtonClicked()
{
    QColorDialog dialog(this);

    QColor color(255,255,255);
    dialog.setCurrentColor(color);

    if (dialog.exec() == QDialog::Accepted)
    {
        color = dialog.selectedColor();
        ui->SpecularColorButton->setStyleSheet(QString("QToolButton {background-color: rgb(%1,%2,%3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
     // Update attributes.
    }

    this->raise();
}

// Scaling/Size
void QvisMeshPropertiesDialog::ScalingSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ScalingSpinBox);
    ui->ScalingSpinBox->setValue(double(value)/100.0);
    ui->ScalingSpinBox->setFocus();

    // Update attributes.
}

void QvisMeshPropertiesDialog::ScalingValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ScalingSlider);
    ui->ScalingSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisMeshPropertiesDialog::SizeLimitToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->SizeLimitSpinBox);
    const QSignalBlocker blocker1(ui->SizeLimitSlider);
    ui->SizeLimitSpinBox->setEnabled(checked);
    ui->SizeLimitSlider->setEnabled(checked);

    // Update attributes.
}
void QvisMeshPropertiesDialog::SizeLimitSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->SizeLimitSpinBox);
    ui->SizeLimitSpinBox->setValue(value);
    ui->SizeLimitSpinBox->setFocus();

    // Update attributes.
}

void QvisMeshPropertiesDialog::SizeLimitValueChanged(int value)
{
    const QSignalBlocker blocker(ui->SizeLimitSlider);
    ui->SizeLimitSlider->setValue(value);

    // Update attributes.
}

// Rendering
void QvisMeshPropertiesDialog::ShininessSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShininessSpinBox);
    ui->ShininessSpinBox->setValue(value);
    ui->SizeLimitSpinBox->setFocus();

    // Update attributes.
}

void QvisMeshPropertiesDialog::ShininessValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ShininessSlider);
    ui->ShininessSlider->setValue(value);

    // Update attributes.
}

void QvisMeshPropertiesDialog::TransparencySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->TransparencySpinBox);
    ui->TransparencySpinBox->setValue(double(value)/100.0);
    ui->TransparencySpinBox->setFocus();

    // Update attributes.
}

void QvisMeshPropertiesDialog::TransparencyValueChanged(double value)
{
    const QSignalBlocker blocker(ui->TransparencySlider);
    ui->TransparencySlider->setValue(value*100.0);

    // Update attributes.
}

void QvisMeshPropertiesDialog::ShadowToggled(bool checked)
{
    const QSignalBlocker blocker0(ui->ShadowSpinBox);
    const QSignalBlocker blocker1(ui->ShadowSlider);
    ui->ShadowSpinBox->setEnabled(checked);
    ui->ShadowSlider->setEnabled(checked);

    // Update attributes.
}

void QvisMeshPropertiesDialog::ShadowSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ShadowSpinBox);
    ui->ShadowSpinBox->setValue(double(value)/100.0);
    ui->ShadowSpinBox->setFocus();

    // Update attributes.
}

void QvisMeshPropertiesDialog::ShadowValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ShadowSlider);
    ui->ShadowSlider->setValue(value*100.0);

    // Update attributes.
}

void QvisMeshPropertiesDialog::LightingToggled(bool checked)
{
    // Update attributes.
}

