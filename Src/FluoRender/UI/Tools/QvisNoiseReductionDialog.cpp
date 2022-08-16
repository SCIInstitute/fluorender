#include "QvisNoiseReductionDialog.h"
#include "ui_QvisNoiseReductionDialog.h"

QvisNoiseReductionDialog::QvisNoiseReductionDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisNoiseReductionDialog)
{
    ui->setupUi(this);
}

QvisNoiseReductionDialog::~QvisNoiseReductionDialog()
{
    delete ui;
}

void QvisNoiseReductionDialog::updateWindow(bool doAll)
{

}

void QvisNoiseReductionDialog::ThresholdSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ThresholdSpinBox);
    ui->ThresholdSpinBox->setValue(value);
    ui->ThresholdSpinBox->setFocus();

    sendNotification();
}

void QvisNoiseReductionDialog::ThresholdValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ThresholdSlider);
    ui->ThresholdSlider->setValue(value);

    sendNotification();
}

void QvisNoiseReductionDialog::VoxelSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->VoxelSizeSpinBox);
    ui->VoxelSizeSpinBox->setValue(double(value)/100.0);
    ui->VoxelSizeSpinBox->setFocus();

    sendNotification();
}

void QvisNoiseReductionDialog::VoxelSizeValueChanged(double value)
{
    const QSignalBlocker blocker(ui->VoxelSizeSlider);
    ui->VoxelSizeSlider->setValue(value*100.0);

    sendNotification();
}

void QvisNoiseReductionDialog::SlectedRegionOnlyToggled(bool checked)
{

}

void QvisNoiseReductionDialog::EnhanceSelectionToggled(bool checked)
{

}

void QvisNoiseReductionDialog::EraseButtonClicked()
{

}

void QvisNoiseReductionDialog::PreviewButtonClicked()
{

}
