#ifndef QVISPREFERENCESDIALOG_H
#define QVISPREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class QvisPreferencesDialog;
}

class QvisPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QvisPreferencesDialog(QWidget *parent = nullptr);
    ~QvisPreferencesDialog();

signals:
    void animationSliderModeChanged(int value);
    void animationPauseValueChanged(int value);

private slots:
    // Project
    void CompressDataToggled(bool checked);
    void SaveProjectToggled(bool checked);
    void TextrueSizeToggled(bool checked);
    void TextureMaxSizeValueChanged(int value);
    void PaintUndoRedoValueChanged(int value);
    // Performance
    void ReduceSampleRateToggled(bool checked);
    void DataStreamingToggled(bool checked);
    void UpdateOrderBackToFrontButtonClicked();
    void UpdateOrderFrontToBackButtonClicked();
    void GraphicsMemorySliderChanged(int value);
    void GraphicsMemoryValueChanged(int value);
    void LargeDataSizeSliderChanged(int value);
    void LargeDataSizeValueChanged(int value);
    void BrickSizeSliderChanged(int value);
    void BrickSizeValueChanged(int value);
    void ResponseTimeSliderChanged(int value);
    void ResponseTimeValueChanged(int value);
    void DetailLevelOffsetSliderChanged(int value);
    void DetailLevelOffsetValueChanged(int value);
    void AnimationSliderModeChanged(int value);
    void AnimationPauseValueChanged(int value);

    // File Format
    void VoxelSizeFromFirstDatasetToggled(bool checked);
    void DefaultColor0ComboBoxIndexChanged(int index);
    void DefaultColor1ComboBoxIndexChanged(int index);
    void DefaultColor2ComboBoxIndexChanged(int index);
    void DefaultColor3ComboBoxIndexChanged(int index);
    // Java - ImageJ
    void JavaImageJButtonToggled(bool checked);
    void JavaFijiButtonToggled(bool checked);
    void JavaPathToAppFileTextEditFinished();
    void JavaPathToAppFileBrowseButtonClicked();
    void JavaPathToLibFileTextEditFinished();
    void JavaPathToLibFileBrowseButtonClicked();
    void JavaPathToJarFileTextEditFinished();
    void JavaPathToJarFileBrowseButtonClicked();

private:
    Ui::QvisPreferencesDialog *ui;
};

#endif // QVISPREFERENCESDIALOG_H
