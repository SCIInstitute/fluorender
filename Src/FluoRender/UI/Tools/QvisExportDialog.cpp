#include "QvisExportDialog.h"
#include "ui_QvisExportDialog.h"

#include "QvisMoviePreferencesDialog.h"
#include "MoviePreferencesAttributes.h"

QvisExportDialog::QvisExportDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisExportDialog)
{
    ui->setupUi(this);
}

QvisExportDialog::~QvisExportDialog()
{
    delete ui;
}


void QvisExportDialog::setMoviePreferencesDialog(QvisMoviePreferencesDialog *dialog)
{
    mMoviePreferencesDialog = dialog;
}

void QvisExportDialog::setMoviePreferencesAttributes(MoviePreferencesAttributes * a)
{
    mMoviePreferencesAttributes = a;
    mMoviePreferencesDialog->setAttributes(mMoviePreferencesAttributes);
}

void QvisExportDialog::updateWindow(bool doAll)
{

}

void QvisExportDialog::ViewWindowListChanged(const QStringList &viewWindows)
{
    QString current = ui->CaptureComboBox->currentText();

    ui->CaptureComboBox->clear();

    ui->CaptureComboBox->addItems(viewWindows);

    ui->CaptureComboBox->setCurrentText(current);
}

// Main
void QvisExportDialog::FramesPerSecondValueChanged(int value)
{

}

void QvisExportDialog::CaptureIndexChanged(int index)
{

}

void QvisExportDialog::PlayButtonClicked()
{

}

void QvisExportDialog::RewindButtonClicked()
{

}

void QvisExportDialog::FrameTimeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->FrameTimeSpinBox);
    ui->FrameTimeSpinBox->setValue(double(value)/100.0);
    ui->FrameTimeSpinBox->setFocus();
}

void QvisExportDialog::FrameTimeValueChanged(double value)
{
    const QSignalBlocker blocker(ui->FrameTimeSlider);
    ui->FrameTimeSlider->setValue(value*100.0);
}

void QvisExportDialog::SaveButtonClicked()
{
    if (mMoviePreferencesDialog->exec() == QDialog::Accepted)
    {

    }
}

// Basic
void QvisExportDialog::RotationToggled(bool checked)
{
    ui->AxisXButton->setEnabled(checked);
    ui->AxisYButton->setEnabled(checked);
    ui->AxisYButton->setEnabled(checked);
    ui->RotationDegreesSpinBox->setEnabled(checked);
    ui->RotationInterpolationComboBox->setEnabled(checked);
}

void QvisExportDialog::AxisXButtonClicked()
{

}

void QvisExportDialog::AxisYButtonClicked()
{

}

void QvisExportDialog::AxisZButtonClicked()
{

}

void QvisExportDialog::RotationDegreesValueChanged(int value)
{

}

void QvisExportDialog::RotationInterpolationIndexChanged(int index)
{

}

void QvisExportDialog::FrameTimeSquenceButtonClicked()
{

}

void QvisExportDialog::FrameBatchProcessButtonClicked()
{

}

void QvisExportDialog::FrameStartValueChanged(int value)
{

}

void QvisExportDialog::FrameEndValueChanged(int value)
{

}

void QvisExportDialog::FrameCurrentValueChanged(int value)
{

}

void QvisExportDialog::MovieTimeValueChanged(double value)
{

}

// Advanced
void QvisExportDialog::KeyFrameTableWidgetCellChanged(int row, int column)
{

}

void QvisExportDialog::KeyFrameOffsetValueChanged(int value)
{

}

void QvisExportDialog::KeyFrameInterpolationIndexChanged(int index)
{

}

void QvisExportDialog::KeyFrameAddButtonClicked()
{

}

void QvisExportDialog::KeyFrameDeleteButtonClicked()
{

}

void QvisExportDialog::KeyFrameDeleteAllButtonClicked()
{

}

void QvisExportDialog::KeyFrameLockTargetViewToggled(bool checked)
{
    ui->KeyFrameLockTargetViewComboBox->setEnabled(checked);
    ui->KeyFrameLockTargetViewSetButton->setEnabled(checked);
}

void QvisExportDialog::KeyFrameLockTargetViewIndexChanged(int index)
{

}

void QvisExportDialog::KeyFrameLockTargetViewSetButtonClicked()
{

}

// Auto Keyframe
void QvisExportDialog::AutoKeyFrameTypeIndexChanged(int index)
{

}

void QvisExportDialog::AutoKeyFrameGenerateButtonClicked()
{

}

// Cropping
void QvisExportDialog::CroppingToggled(bool checked)
{
    ui->CropXSpinBox->setEnabled(checked);
    ui->CropYSpinBox->setEnabled(checked);
    ui->CropWidthSpinBox->setEnabled(checked);
    ui->CropHeightSpinBox->setEnabled(checked);
}

void QvisExportDialog::CropXValueChanged(int value)
{

}

void QvisExportDialog::CropYValueChanged(int value)
{

}

void QvisExportDialog::CropWidthValueChanged(int value)
{

}

void QvisExportDialog::CropHeightValueChanged(int value)
{

}

void QvisExportDialog::CroppingResetButtonClicked()
{

}

// Scripting
void QvisExportDialog::ScriptingToggled(bool checked)
{
    ui->ScriptFileTextEdit->setEnabled(checked);
    ui->ScriptFileBrowseButton->setEnabled(checked);
    ui->ScriptFileTableWidget->setEnabled(checked);
}

void QvisExportDialog::ScriptFileTextEditFinished()
{

}

void QvisExportDialog::ScriptFileBrowseButtonClicked()
{

}
