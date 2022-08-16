#ifndef QVISMEASUREMENTDIALOG_H
#define QVISMEASUREMENTDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisMeasurementDialog;
}

class QvisMeasurementDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisMeasurementDialog(QWidget *parent = nullptr);
    ~QvisMeasurementDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    // First row
    void LocatorButtonClicked();
    void ProbeButtonClicked();
    void LineButtonClicked();
    void ProtractorButtonClicked();
    void EllipseButtonClicked();
    void MultiPointLineButtonClicked();
    void PencilButtonClicked();
    void GrowButtonClicked();
    // Second row
    void MoveButtonClicked();
    void EditButtonClicked();
    void DeleteButtonClicked();
    void FlipButtonClicked();
    void AverageButtonClicked();
    void PruneButtonClicked();
    void LockButtonClicked();
    void RelaxButtonClicked();
    // Third row
    void DeleteRulerButtonClicked();
    void DeleteAllRulersButtonClicked();
    void ProfileButtonClicked();
    void DistanceButtonClicked();
    void ProjectButtonClicked();
    void ExportButtonClicked();

    // Settings
    void ZDepthViewPlaneButtonClicked();
    void ZDepthMaxIntensityButtonClicked();
    void ZDepthAccumIntensityButtonClicked();
    void TransientToggled(bool checked);
    void UseVolumePropertiesToggled(bool checked);
    void ComputeDeltaFFToggled(bool checked);
    void AutoRelaxToggled(bool checked);
    void ConstraintIndexChanged(int index);
    void ExInRatioValueChanged(double value);

    // Ruler list
    void NewGroupButtonClicked();
    void GroupIDValueChanged(int value);
    void ChangeGroupButtonClicked();
    void SelectGroupButtonClicked();
    void DisplayGroupButtonClicked();

    // Align
    void MonoAxisXPosButtonClicked();
    void MonoAxisXNegButtonClicked();
    void MonoAxisYPosButtonClicked();
    void MonoAxisYNegButtonClicked();
    void MonoAxisZPosButtonClicked();
    void MonoAxisZNegButtonClicked();
    void TriAxisXYZButtonClicked();
    void TriAxisXZYButtonClicked();
    void TriAxisYXZButtonClicked();
    void TriAxisYZXButtonClicked();
    void TriAxisZXYButtonClicked();
    void TriAxisZYXButtonClicked();
    void MoveToCenterToggled(bool checked);

private:
    Ui::QvisMeasurementDialog *ui;

};

#endif // QVISMEASUREMENTDIALOG_H
