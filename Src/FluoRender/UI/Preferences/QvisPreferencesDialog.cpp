#include "QvisPreferencesDialog.h"
#include "ui_QvisPreferencesDialog.h"

#include <QFileDialog>

QvisPreferencesDialog::QvisPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QvisPreferencesDialog)
{
    ui->setupUi(this);

    JavaImageJButtonToggled( ui->JavaImageJButton->isChecked());
    JavaFijiButtonToggled(ui->JavaFijiButton->isChecked());
}

QvisPreferencesDialog::~QvisPreferencesDialog()
{
    delete ui;
}

// Project
void QvisPreferencesDialog::CompressDataToggled(bool checked)
{

}

void QvisPreferencesDialog::SaveProjectToggled(bool checked)
{

}

void QvisPreferencesDialog::TextrueSizeToggled(bool checked)
{
    // Not really needed as the GroupBox enables/disables.
    ui->TextureMaxSizeSpinBox->setEnabled(checked);
    ui->TextureSizeNoteLabel->setEnabled(checked);
}

void QvisPreferencesDialog::TextureMaxSizeValueChanged(int value)
{

}

void QvisPreferencesDialog::PaintUndoRedoValueChanged(int value)
{

}

// Performance
void QvisPreferencesDialog::ReduceSampleRateToggled(bool checked)
{

}

void QvisPreferencesDialog::DataStreamingToggled(bool checked)
{
    // Not really needed as the GroupBox enables/disables.
    ui->UpdateOrderLabel->setEnabled(checked);
    ui->UpdateOrderBackToFrontButton->setEnabled(checked);
    ui->UpdateOrderFrontToBackButton->setEnabled(checked);

    ui->GraphicsMemoryLabel->setEnabled(checked);
    ui->GraphicsMemorySlider->setEnabled(checked);
    ui->GraphicsMemorySpinBox->setEnabled(checked);

    ui->LargeDataSizeLabel->setEnabled(checked);
    ui->LargeDataSizeSlider->setEnabled(checked);
    ui->LargeDataSizeSpinBox->setEnabled(checked);

    ui->BrickSizeLabel->setEnabled(checked);
    ui->BrickSizeSlider->setEnabled(checked);
    ui->BrickSizeSpinBox->setEnabled(checked);

    ui->ResponseTimeLabel->setEnabled(checked);
    ui->ResponseTimeSlider->setEnabled(checked);
    ui->ResponseTimeSpinBox->setEnabled(checked);

    ui->DetailLevelOffsetLabel->setEnabled(checked);
    ui->DetailLevelOffsetSlider->setEnabled(checked);
    ui->DetailLevelOffsetSpinBox->setEnabled(checked);

    ui->DataStreamingNoteLabel->setEnabled(checked);
}

void QvisPreferencesDialog::UpdateOrderBackToFrontButtonClicked()
{

}

void QvisPreferencesDialog::UpdateOrderFrontToBackButtonClicked()
{

}

void QvisPreferencesDialog::GraphicsMemorySliderChanged(int value)
{
    const QSignalBlocker blocker(ui->GraphicsMemorySpinBox);
    ui->GraphicsMemorySpinBox->setValue(value);
    ui->GraphicsMemorySpinBox->setFocus();
}

void QvisPreferencesDialog::GraphicsMemoryValueChanged(int value)
{
    const QSignalBlocker blocker(ui->GraphicsMemorySlider);
    ui->GraphicsMemorySlider->setValue(value);
}

void QvisPreferencesDialog::LargeDataSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->LargeDataSizeSpinBox);
    ui->LargeDataSizeSpinBox->setValue(value);
    ui->LargeDataSizeSpinBox->setFocus();
}

void QvisPreferencesDialog::LargeDataSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->LargeDataSizeSlider);
    ui->LargeDataSizeSlider->setValue(value);
}

void QvisPreferencesDialog::BrickSizeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->BrickSizeSpinBox);
    ui->BrickSizeSpinBox->setValue(value);
    ui->BrickSizeSpinBox->setFocus();
}

void QvisPreferencesDialog::BrickSizeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->BrickSizeSlider);
    ui->BrickSizeSlider->setValue(value);
}

void QvisPreferencesDialog::ResponseTimeSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ResponseTimeSpinBox);
    ui->ResponseTimeSpinBox->setValue(value);
    ui->ResponseTimeSpinBox->setFocus();
}

void QvisPreferencesDialog::ResponseTimeValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ResponseTimeSlider);
    ui->ResponseTimeSlider->setValue(value);
}

void QvisPreferencesDialog::DetailLevelOffsetSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->DetailLevelOffsetSpinBox);
    ui->DetailLevelOffsetSpinBox->setValue(value);
    ui->DetailLevelOffsetSpinBox->setFocus();
}

void QvisPreferencesDialog::DetailLevelOffsetValueChanged(int value)
{
    const QSignalBlocker blocker(ui->DetailLevelOffsetSlider);
    ui->DetailLevelOffsetSlider->setValue(value);
}

void QvisPreferencesDialog::AnimationSliderModeChanged(int value)
{
    emit animationSliderModeChanged(value);
}

void QvisPreferencesDialog::AnimationPauseValueChanged(int value)
{
    emit animationPauseValueChanged(value);
}

// File Format
void QvisPreferencesDialog::VoxelSizeFromFirstDatasetToggled(bool checked)
{

}

void QvisPreferencesDialog::DefaultColor0ComboBoxIndexChanged(int index)
{

}

void QvisPreferencesDialog::DefaultColor1ComboBoxIndexChanged(int index)
{

}

void QvisPreferencesDialog::DefaultColor2ComboBoxIndexChanged(int index)
{

}

void QvisPreferencesDialog::DefaultColor3ComboBoxIndexChanged(int index)
{

}

// Java - ImageJ
void QvisPreferencesDialog::JavaImageJButtonToggled(bool checked)
{
    if( checked )
    {
#ifdef __APPLE__
        ui->JavaPathToAppFileLabel->setText("Path to folder: Imagej.app");
        ui->JavaPathToLibFileLabel->setText("Path to file: libjvm.dylib");
#elif defined(WIN32)
        ui->JavaPathToAppFileLabel->setText("Path to file: Imagej.exe");
        ui->JavaPathToLibFileLabel->setText("Path to file: libjvm.dll");
#else
        ui->JavaPathToAppFileLabel->setText("Path to file: Imagej");
        ui->JavaPathToLibFileLabel->setText("Path to file: libjvm.so");
#endif

        ui->JavaPathToJarFileLabel->setText("Path to bioformats_package.jar");

        ui->JavaPathToLibFileLabel->setEnabled(checked);
        ui->JavaPathToLibFileLabel->setHidden(!checked);
        ui->JavaPathToJarFileLabel->setEnabled(checked);
        ui->JavaPathToJarFileLabel->setHidden(!checked);

        ui->JavaPathToLibFileTextEdit->setEnabled(checked);
        ui->JavaPathToLibFileTextEdit->setHidden(!checked);
        ui->JavaPathToJarFileTextEdit->setEnabled(checked);
        ui->JavaPathToJarFileTextEdit->setHidden(!checked);

        ui->JavaPathToLibFileBrowseButton->setEnabled(checked);
        ui->JavaPathToLibFileBrowseButton->setHidden(!checked);
        ui->JavaPathToJarFileBrowseButton->setEnabled(checked);
        ui->JavaPathToJarFileBrowseButton->setHidden(!checked);
    }
}

void QvisPreferencesDialog::JavaFijiButtonToggled(bool checked)
{
    if( checked )
    {
        ui->JavaPathToLibFileLabel->setEnabled(!checked);
        ui->JavaPathToLibFileLabel->setHidden(checked);
        ui->JavaPathToJarFileLabel->setEnabled(!checked);
        ui->JavaPathToJarFileLabel->setHidden(checked);

        ui->JavaPathToLibFileTextEdit->setEnabled(!checked);
        ui->JavaPathToLibFileTextEdit->setHidden(checked);
        ui->JavaPathToJarFileTextEdit->setEnabled(!checked);
        ui->JavaPathToJarFileTextEdit->setHidden(checked);

        ui->JavaPathToLibFileBrowseButton->setEnabled(!checked);
        ui->JavaPathToLibFileBrowseButton->setHidden(checked);
        ui->JavaPathToJarFileBrowseButton->setEnabled(!checked);
        ui->JavaPathToJarFileBrowseButton->setHidden(checked);
    }
}

void QvisPreferencesDialog::JavaPathToAppFileTextEditFinished()
{
    ui->JavaPathToAppFileTextEdit->text();
}

void QvisPreferencesDialog::JavaPathToAppFileBrowseButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    //dialog.setDirectory();

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList fileNames = dialog.selectedFiles();

        if( fileNames.size() == 1)
        {
            const QSignalBlocker blocker(ui->JavaPathToAppFileTextEdit);
            ui->JavaPathToAppFileTextEdit->setText(fileNames[0]);
            // Update attributes.
        }
    }

    this->raise();
}

void QvisPreferencesDialog::JavaPathToLibFileTextEditFinished()
{

}

void QvisPreferencesDialog::JavaPathToLibFileBrowseButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    //dialog.setDirectory();

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList fileNames = dialog.selectedFiles();

        if( fileNames.size() == 1)
        {
            const QSignalBlocker blocker(ui->JavaPathToLibFileTextEdit);
            ui->JavaPathToLibFileTextEdit->setText(fileNames[0]);
            // Update attributes.
        }
    }

    this->raise();
}

void QvisPreferencesDialog::JavaPathToJarFileTextEditFinished()
{

}

void QvisPreferencesDialog::JavaPathToJarFileBrowseButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    //dialog.setDirectory();

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList fileNames = dialog.selectedFiles();

        if( fileNames.size() == 1)
        {
            const QSignalBlocker blocker(ui->JavaPathToJarFileTextEdit);
            ui->JavaPathToJarFileTextEdit->setText(fileNames[0]);
            // Update attributes.
        }
    }

    this->raise();
}

