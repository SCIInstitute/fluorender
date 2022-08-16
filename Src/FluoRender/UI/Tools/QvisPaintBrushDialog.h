#ifndef QVISPAINTBRUSHDIALOG_H
#define QVISPAINTBRUSHDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisPaintBrushDialog;
}

class QvisPaintBrushDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisPaintBrushDialog(QWidget *parent = nullptr);
    ~QvisPaintBrushDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    // First row
    void UndoButtonClicked();
    void RedoButtonClicked();
    void GrowButtonClicked();
    void SelectButtonClicked();
    void DifuseButtonClicked();
    void SolidButtonClicked();
    void UnselectButtonClicked();
    void EraseButtonClicked();
    void ExtractButtonClicked();
    void ResetButtonClicked();
    // Second row
    void CopyButtonClicked();
    void DataButtonClicked();
    void PasteButtonClicked();
    void MergeButtonClicked();
    void ExcludeButtonClicked();
    void IntersectButtonClicked();
    // Selection Settings
    void AutoClearToggled(bool checked);
    void EdgeDetectToggled(bool checked);
    void VisibleOnlyToggled(bool checked);
    void CrossBricksToggled(bool checked);
    void ApplyToGroupToggled(bool checked);
    void ThresholdSliderChanged(int value);
    void ThresholdValueChanged(double value);
    void EdgeStrengthSliderChanged(int value);
    void EdgeStrengthValueChanged(double value);
    // Brush Properties
    void GrowthCurrentIndexChanged(int index);
    void DependentCurrentIndexChanged(int index);
    void CenterSizeSliderChanged(int value);
    void CenterSizeValueChanged(int value);
    void GrowSizeToggled(bool checked);
    void GrowSizeSliderChanged(int value);
    void GrowSizeValueChanged(int value);
    // Align
    void TriAxisXYZButtonClicked();
    void TriAxisXZYButtonClicked();
    void TriAxisYXZButtonClicked();
    void TriAxisYZXButtonClicked();
    void TriAxisZXYButtonClicked();
    void TriAxisZYXButtonClicked();
    void MoveToCenterToggled(bool checked);
    // Output
    void GetSelectedSizeButtonClicked();
    void AutoUpdateToggled(bool checked);
    void HoldHistoryToggled(bool checked);
    void HoldHistoryValueChanged(int value);
    void ClearHistoryButtonClicked();

private:
    Ui::QvisPaintBrushDialog *ui;
};

#endif // QVISPAINTBRUSHDIALOG_H
