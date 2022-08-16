#include "QvisCapturePreferencesDialog.h"
#include "ui_QvisCapturePreferencesDialog.h"

#include "CapturePreferencesAttributes.h"

#include <QFileDialog>
#include <QMessageBox>

#include <filesystem>
#include <iostream>

QvisCapturePreferencesDialog::QvisCapturePreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QvisCapturePreferencesDialog)
{
    ui->setupUi(this);

    // The tmp atts are used to store the new values. Once "ok" is pressed
    // the actual attributes are updated.
    tmpAtts = new CapturePreferencesAttributes();
}

QvisCapturePreferencesDialog::~QvisCapturePreferencesDialog()
{
    delete ui;

    delete tmpAtts;
}

const CapturePreferencesAttributes * QvisCapturePreferencesDialog::attributes() const
{
    return atts;
}

void QvisCapturePreferencesDialog::setAttributes(CapturePreferencesAttributes * a)
{
    atts = a;

    // The tmp atts are used to store the new values. Once "ok" is pressed
    // the actual attributes are updated.
    *tmpAtts = *atts;

    const QSignalBlocker blocker(this);

    ui->FileNameText->setText(atts->filename.c_str());
    ui->DirectoryText->setText(atts->directory.c_str());
    ui->FileNameFamilyCheckBox->setChecked(atts->family);
    ui->EmbedInProjectFolderCheckBox->setChecked(atts->embed);

    ui->LZWCompressionCheckBox->setChecked(atts->LZWCompression);
    ui->SaveAlphaChannelCheckBox->setChecked(atts->saveAlphaChannel);
    ui->SaveFloatChannelCheckBox->setChecked(atts->saveFloatChannel);

    ui->ScaleOutputImageGroupBox->setChecked(atts->scaleOutputImage);
    ui->ScaleOutputImageSpinBox->setValue(atts->outputImageScale);
}

// The project directory get set once there is an openned or saved project.
const QString QvisCapturePreferencesDialog::currentProjectDirectory() const
{
    return mCurrentProjectDirectory;
}

void QvisCapturePreferencesDialog::setCurrentProjectDirectory(const QString &directory)
{
    mCurrentProjectDirectory = directory;

    if(ui->EmbedInProjectFolderCheckBox->isChecked())
        EmbedInProjectFolderToggled(true);
}

void QvisCapturePreferencesDialog::FileNameTextChanged(const QString &text)
{
    tmpAtts->filename = text.toStdString();
}

void QvisCapturePreferencesDialog::FileNameFamilyToggled(bool checked)
{
    tmpAtts->family = checked;
}

void QvisCapturePreferencesDialog::EmbedInProjectFolderToggled(bool checked)
{
    tmpAtts->embed = checked;

    ui->DirectoryLabel ->setEnabled(!checked);
    ui->DirectoryText  ->setEnabled(!checked);
    ui->DirectoryButton->setEnabled(!checked);

    // TO DO Get the current project directory.
    if(checked)
    {
        if(mCurrentProjectDirectory.isEmpty())
        {
            QMessageBox dialog;
            dialog.setText("No current project directory set.");
            dialog.exec();

            QSignalBlocker blocker(ui->EmbedInProjectFolderCheckBox);

            ui->EmbedInProjectFolderCheckBox->setChecked(!checked);
            ui->DirectoryLabel ->setEnabled(checked);
            ui->DirectoryText  ->setEnabled(checked);
            ui->DirectoryButton->setEnabled(checked);
        }
        else
        {
            ui->DirectoryText->setText(mCurrentProjectDirectory);
        }
    }
}

bool QvisCapturePreferencesDialog::DirectoryTextReturnPressed()
{
    QString directory = ui->DirectoryText->text();

    // The name exists but is not a directory.
    if(std::filesystem::exists(directory.toStdString()) && !std::filesystem::is_directory(directory.toStdString()))
    {
        QMessageBox dialog;
        dialog.setText("Cannot write to " + directory + ".");
        dialog.setInformativeText("Though it exists it is not a directory.");
        dialog.setStandardButtons(QMessageBox::Ok);
        dialog.exec();
        ui->DirectoryText->setText(tmpAtts->directory.c_str());
        return false;
    }

    // If the directory does not exist create it.
    if(!std::filesystem::exists(directory.toStdString()))
    {
        QMessageBox dialog;
        dialog.setText("The directory " + directory + " "
                       "does not exist.");
        dialog.setInformativeText("Do you want to create the directory?");
        dialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        if(dialog.exec() != QMessageBox::Ok)
        {
            ui->DirectoryText->setText(tmpAtts->directory.c_str());
            return false;
        }

        if(!std::filesystem::create_directories(directory.toStdString()))
        {
            QMessageBox dialog;
            dialog.setText("Could not create the directory" + directory);
            dialog.setStandardButtons(QMessageBox::Ok);
            dialog.exec();
            ui->DirectoryText->setText(tmpAtts->directory.c_str());
            return false;
        }
    }

    tmpAtts->directory = directory.toStdString();
    return true;
}

void QvisCapturePreferencesDialog::DirectoryButtonClicked()
{
    QFileDialog dialog(this, "Output Directory");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(ui->DirectoryText->text());

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList directoryNames = dialog.selectedFiles();

        if(directoryNames.empty() || directoryNames.size() > 1)
            return;

        QSignalBlocker blocker(ui->FileNameText);

        ui->DirectoryText->setText(directoryNames[0]);
    }
}

void QvisCapturePreferencesDialog::LZWCompressionToggled(bool checked)
{
    tmpAtts->LZWCompression = checked;
}

void QvisCapturePreferencesDialog::SaveAlphaChannelToggled(bool checked)
{
    tmpAtts->saveAlphaChannel = checked;
}

void QvisCapturePreferencesDialog::SaveFloatChannelToggled(bool checked)
{
    tmpAtts->saveFloatChannel = checked;
}

void QvisCapturePreferencesDialog::ScaleOutputImageToggled(bool checked)
{
    ui->ScaleOutputImageSlider->setEnabled(checked);
    ui->ScaleOutputImageSpinBox->setEnabled(checked);

    if(!checked)
        ui->ScaleOutputImageSpinBox->setValue(1.0);

    tmpAtts->scaleOutputImage = checked;
}

void QvisCapturePreferencesDialog::ScaleOutputImageSliderChanged(int value)
{
    QSignalBlocker blocker(ui->ScaleOutputImageSpinBox);

    ui->ScaleOutputImageSpinBox->setValue(double(value)/10.0);

    tmpAtts->outputImageScale = double(value) / 10.0;
}

void QvisCapturePreferencesDialog::ScaleOutputImageSpinBoxChanged(double value)
{
    QSignalBlocker blocker(ui->ScaleOutputImageSlider);

    ui->ScaleOutputImageSlider->setValue(value*10);

    tmpAtts->outputImageScale = value;
}

void QvisCapturePreferencesDialog::CancelButtonClicked()
{
    // Restore the attributes.
    *tmpAtts = *atts;

    const QSignalBlocker blocker(this);

    ui->FileNameText->setText(atts->filename.c_str());
    ui->DirectoryText->setText(atts->directory.c_str());
    ui->FileNameFamilyCheckBox->setChecked(atts->family);
    ui->EmbedInProjectFolderCheckBox->setChecked(atts->embed);

    ui->LZWCompressionCheckBox->setChecked(atts->LZWCompression);
    ui->SaveAlphaChannelCheckBox->setChecked(atts->saveAlphaChannel);
    ui->SaveFloatChannelCheckBox->setChecked(atts->saveFloatChannel);

    ui->ScaleOutputImageGroupBox->setChecked(atts->scaleOutputImage);
    ui->ScaleOutputImageSpinBox->setValue(atts->outputImageScale);

    this->close();
}

void QvisCapturePreferencesDialog::OkButtonClicked()
{
    // Check for valid directory which happens when a return is pressed. If it
    // is not valid do not close the window so the user can correct it.
    if(DirectoryTextReturnPressed())
    {
        // Update the attributes and close the window.
        *atts = *tmpAtts;

        this->close();
    }
}
