#include "QvisConvertDialog.h"
#include "ui_QvisConvertDialog.h"

QvisConvertDialog::QvisConvertDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisConvertDialog)
{
    ui->setupUi(this);
}

QvisConvertDialog::~QvisConvertDialog()
{
    delete ui;
}

void QvisConvertDialog::updateWindow(bool doAll)
{

}

void QvisConvertDialog::ThresholdSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ThresholdSpinBox);
    ui->ThresholdSpinBox->setValue(double(value)/100.0);
    ui->ThresholdSpinBox->setFocus();
}

void QvisConvertDialog::ThresholdValueChanged(double value)
{
    const QSignalBlocker blocker(ui->ThresholdSlider);
    ui->ThresholdSlider->setValue(value*100.0);
}

void QvisConvertDialog::DownSampleXYSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DownSampleXYSpinBox);
    ui->DownSampleXYSpinBox->setValue(value);
    ui->DownSampleXYSpinBox->setFocus();
}

void QvisConvertDialog::DownSampleXYValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DownSampleXYSlider);
    ui->DownSampleXYSlider->setValue(value);
}

void QvisConvertDialog::DownSampleZSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DownSampleZSpinBox);
    ui->DownSampleZSpinBox->setValue(value);
    ui->DownSampleZSpinBox->setFocus();
}

void QvisConvertDialog::DownSampleZValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DownSampleZSlider);
    ui->DownSampleZSlider->setValue(value);
}

void QvisConvertDialog::UseTransferFunctionToggled(bool value)
{

}

void QvisConvertDialog::SelectedOnlyToggled(bool value)
{

}

void QvisConvertDialog::WeldVerticesToggled(bool value)
{

}

void QvisConvertDialog::ConvertClicked()
{
    sendNotification();
}

