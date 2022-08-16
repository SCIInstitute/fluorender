#include "QvisMoviePreferencesDialog.h"
#include "ui_QvisMoviePreferencesDialog.h"

#include "MoviePreferencesAttributes.h"

#include <QFileDialog>
#include <QMessageBox>

#include <filesystem>
#include <iostream>

QvisMoviePreferencesDialog::QvisMoviePreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QvisMoviePreferencesDialog)
{
    ui->setupUi(this);

    // The tmp atts are used to store the new values. Once "ok" is pressed
    // the actual attributes are updated.
    tmpAtts = new MoviePreferencesAttributes();

    this->FileTypeIndexChanged(ui->FileTypeComboBox->currentIndex());
}

QvisMoviePreferencesDialog::~QvisMoviePreferencesDialog()
{
    delete ui;
}

const MoviePreferencesAttributes * QvisMoviePreferencesDialog::attributes() const
{
    return atts;
}

void QvisMoviePreferencesDialog::setAttributes(MoviePreferencesAttributes * a)
{
    atts = a;

    // The tmp atts are used to store the new values. Once "ok" is pressed
    // the actual attributes are updated.
    *tmpAtts = *atts;

    const QSignalBlocker blocker(this);

    ui->FileNameText->setText(atts->filename.c_str());
    ui->DirectoryText->setText(atts->directory.c_str());
    ui->FileTypeComboBox->setCurrentIndex(atts->fileType);

    ui->LZWCompressionCheckBox->setChecked(atts->LZWCompression);
    ui->SaveAlphaChannelCheckBox->setChecked(atts->saveAlphaChannel);
    ui->SaveFloatChannelCheckBox->setChecked(atts->saveFloatChannel);

    ui->ScaleOutputImageGroupBox->setChecked(atts->scaleOutputImage);
    ui->ScaleOutputImageSpinBox->setValue(atts->outputImageScale);
}

// The project directory get set once there is an openned or saved project.
const QString QvisMoviePreferencesDialog::currentProjectDirectory() const
{
    return mCurrentProjectDirectory;
}

void QvisMoviePreferencesDialog::setCurrentProjectDirectory(const QString &directory)
{
    mCurrentProjectDirectory = directory;

    if(ui->EmbedInProjectFoldercheckBox->isChecked())
        EmbedInProjectFolderToggled(true);
}

void QvisMoviePreferencesDialog::FileNameTextChanged(const QString &text)
{
    tmpAtts->filename = text.toStdString();
}

void QvisMoviePreferencesDialog::EmbedInProjectFolderToggled(bool checked)
{
    ui->EmbedInProjectFoldercheckBox->setChecked(atts->embed);

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

            QSignalBlocker blocker(ui->EmbedInProjectFoldercheckBox);

            ui->EmbedInProjectFoldercheckBox->setChecked(!checked);
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

bool QvisMoviePreferencesDialog::DirectoryTextReturnPressed()
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

void QvisMoviePreferencesDialog::DirectoryButtonClicked()
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

void QvisMoviePreferencesDialog::FileTypeIndexChanged(int index)
{
    tmpAtts->fileType = MoviePreferencesAttributes::OutputType(index);

    if(tmpAtts->fileType == MoviePreferencesAttributes::TIFF)
    {
        ui->BitRateLabel->setHidden(true);
        ui->BitRateSpinBox->setHidden(true);
        ui->BitRateUnitsLabel->setHidden(true);
        ui->BitRateSpacer->setHidden(true);

        ui->EstimatedSizeLabel->setHidden(true);
        ui->EstimatedSizeText->setHidden(true);
        ui->EstimatedSizeUnitsLabel->setHidden(true);
        ui->EstimatedSizeSpacer->setHidden(true);

        ui->LZWCompressionCheckBox->setHidden(false);
        ui->SaveAlphaChannelCheckBox->setHidden(false);
        ui->SaveFloatChannelCheckBox->setHidden(false);
    }
    else // if(tmpAtts->fileType == QvisMoviePreferencesDialog::MOV)
    {
        ui->LZWCompressionCheckBox->setHidden(true);
        ui->SaveAlphaChannelCheckBox->setHidden(true);
        ui->SaveFloatChannelCheckBox->setHidden(true);

        ui->BitRateLabel->setHidden(false);
        ui->BitRateSpinBox->setHidden(false);
        ui->BitRateUnitsLabel->setHidden(false);
        ui->BitRateSpacer->setHidden(false);

        ui->EstimatedSizeLabel->setHidden(false);
        ui->EstimatedSizeText->setHidden(false);
        ui->EstimatedSizeUnitsLabel->setHidden(false);
        ui->EstimatedSizeSpacer->setHidden(false);
    }
}

void QvisMoviePreferencesDialog::LZWCompressionToggled(bool checked)
{
    tmpAtts->LZWCompression = checked;
}

void QvisMoviePreferencesDialog::SaveAlphaChannelToggled(bool checked)
{
    tmpAtts->saveAlphaChannel = checked;
}

void QvisMoviePreferencesDialog::SaveFloatChannelToggled(bool checked)
{
    tmpAtts->saveFloatChannel = checked;
}

void QvisMoviePreferencesDialog::BitRateSpinBoxValueChanged(double value)
{
    tmpAtts->bitrate = value;
}

void QvisMoviePreferencesDialog::ScaleOutputImageToggled(bool checked)
{
    ui->ScaleOutputImageSlider->setEnabled(checked);
    ui->ScaleOutputImageSpinBox->setEnabled(checked);

    if(!checked)
        ui->ScaleOutputImageSpinBox->setValue(1.0);

    tmpAtts->scaleOutputImage = checked;
}

void QvisMoviePreferencesDialog::ScaleOutputImageSliderChanged(int value)
{
    QSignalBlocker blocker(ui->ScaleOutputImageSpinBox);

    ui->ScaleOutputImageSpinBox->setValue(double(value)/10.0);

    tmpAtts->outputImageScale = double(value) / 10.0;
}

void QvisMoviePreferencesDialog::ScaleOutputImageSpinBoxChanged(double value)
{
    QSignalBlocker blocker(ui->ScaleOutputImageSlider);

    ui->ScaleOutputImageSlider->setValue(value*10);

    tmpAtts->outputImageScale = value;
}

void QvisMoviePreferencesDialog::CancelButtonClicked()
{
    // Restore the attributes.
    *tmpAtts = *atts;

    const QSignalBlocker blocker(this);

    ui->FileNameText->setText(atts->filename.c_str());
    ui->DirectoryText->setText(atts->directory.c_str());
    ui->EmbedInProjectFoldercheckBox->setChecked(atts->embed);

    ui->FileTypeComboBox->setCurrentIndex(atts->fileType);

    ui->LZWCompressionCheckBox->setChecked(atts->LZWCompression);
    ui->SaveAlphaChannelCheckBox->setChecked(atts->saveAlphaChannel);
    ui->SaveFloatChannelCheckBox->setChecked(atts->saveFloatChannel);

    ui->BitRateSpinBox->setValue(atts->bitrate);

    ui->ScaleOutputImageGroupBox->setChecked(atts->scaleOutputImage);
    ui->ScaleOutputImageSpinBox->setValue(atts->outputImageScale);

    this->close();
}

void QvisMoviePreferencesDialog::OkButtonClicked()
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
