#ifndef QVISEXPORTDIALOG_H
#define QVISEXPORTDIALOG_H

#include "QvisDialogBase.h"

class QvisMoviePreferencesDialog;
class MoviePreferencesAttributes;

namespace Ui {
class QvisExportDialog;
}

class QvisExportDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisExportDialog(QWidget *parent = nullptr);
    ~QvisExportDialog();

    void updateWindow(bool doAll = true) override;

    void ViewWindowListChanged(const QStringList &viewWindows);

    void setMoviePreferencesDialog(QvisMoviePreferencesDialog *dialog);
    void setMoviePreferencesAttributes(MoviePreferencesAttributes *atts);

private slots:

    // Main
    void FramesPerSecondValueChanged(int value);
    void CaptureIndexChanged(int index);
    void PlayButtonClicked();
    void RewindButtonClicked();
    void FrameTimeSliderChanged(int value);
    void FrameTimeValueChanged(double value);
    void SaveButtonClicked();
    // Basic
    void RotationToggled(bool checked);
    void AxisXButtonClicked();
    void AxisYButtonClicked();
    void AxisZButtonClicked();
    void RotationDegreesValueChanged(int value);
    void RotationInterpolationIndexChanged(int index);
    void FrameTimeSquenceButtonClicked();
    void FrameBatchProcessButtonClicked();
    void FrameStartValueChanged(int value);
    void FrameEndValueChanged(int value);
    void FrameCurrentValueChanged(int value);
    void MovieTimeValueChanged(double value);
    // Advanced
    void KeyFrameTableWidgetCellChanged(int row, int column);
    void KeyFrameOffsetValueChanged(int value);
    void KeyFrameInterpolationIndexChanged(int index);
    void KeyFrameAddButtonClicked();
    void KeyFrameDeleteButtonClicked();
    void KeyFrameDeleteAllButtonClicked();
    void KeyFrameLockTargetViewToggled(bool checked);
    void KeyFrameLockTargetViewIndexChanged(int index);
    void KeyFrameLockTargetViewSetButtonClicked();
    // Auto Keyframe
    void AutoKeyFrameTypeIndexChanged(int index);
    void AutoKeyFrameGenerateButtonClicked();
    // Cropping
    void CroppingToggled(bool checked);
    void CropXValueChanged(int value);
    void CropYValueChanged(int value);
    void CropWidthValueChanged(int value);
    void CropHeightValueChanged(int value);
    void CroppingResetButtonClicked();
    // Scripting
    void ScriptingToggled(bool value);
    void ScriptFileTextEditFinished();
    void ScriptFileBrowseButtonClicked();

private:
    Ui::QvisExportDialog *ui;

    QvisMoviePreferencesDialog *mMoviePreferencesDialog{nullptr};
    MoviePreferencesAttributes *mMoviePreferencesAttributes{nullptr};
};

#endif // QVISEXPORTDIALOG_H
