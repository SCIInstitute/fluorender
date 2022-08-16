#ifndef QVISTRACKINGDIALOG_H
#define QVISTRACKINGDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisTrackingDialog;
}

class QvisTrackingDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisTrackingDialog(QWidget *parent = nullptr);
    ~QvisTrackingDialog();

    void updateWindow(bool doAll = true) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:

    // Main
    void TracksTailToggled(bool checked);
    void TracksLeadToggled(bool checked);
    void TracksSliderChanged(int value);
    void TracksValueChanged(int value);
    void IDListsBackwardButtonClicked();
    void IDListsForewardButtonClicked();
    // Track map
    void TrackMapTextEditFinished();
    void TrackMapLoadButtonClicked();
    void TrackMapSaveButtonClicked();
    void TrackMapSaveAsButtonClicked();
    void TrackMapIterationsValueChanged(int value);
    void TrackMapSizeTresholdValueChanged(int value);
    void TrackMapSimilarityValueChanged(double value);
    void TrackMapContractFactorValueChanged(double value);
    void TrackMapConsistentColorToggled(bool checked);
    void TrackMapTryMergingToggled(bool checked);
    void TrackMapTrySplittingToggled(bool checked);
    void TrackMapGenerateButtonClicked();
    void TrackMapRefineTButtonClicked();
    void TrackMapRefineAllButtonClicked();
    // Selection
    void SelectionTextEditFinished();
    void SelectionAppendButtonClicked();
    void SelectionClearButtonClicked();
    void SelectionReplaceButtonClicked();
    void SelectionFullComponentButtonClicked();
    void SelectionShuffleButtonClicked();
    void SelectionComponentSizeSliderChanged(int value);
    void SelectionComponentSizeValueChanged(int value);
    void SelectionUncertaintySliderChanged(int value);
    void SelectionUncertaintyValueChanged(int value);
    void SelectionUncertaintyButtonClicked();
    // Modify
    void ModifySelectionTextEditFinished();
    void ModifyAppendButtonClicked();
    void ModifyClearButtonClicked();
    void ModifyAssignIDButtonClicked();
    void ModifyAddIDButtonClicked();
    void ModifyReplaceIDButtonClicked();
    void ModifyCombineButtonClicked();
    void ModifySeparateButtonClicked();
    void ModifySegmentButtonClicked();
    void ModifyNumberOfComponentsValueChanged(int value);
    // Linkage
    void LinkageSelectionTextEditFinished();
    void LinkageAppendButtonClicked();
    void LinkageClearButtonClicked();
    void LinkExclusiveButtonClicked();
    void LinkIDsButtonClicked();
    void LinkNewIDsButtonClicked();
    void LinkIsolateButtonClicked();
    void UnlinkIDsButtonClicked();
    // Analysis
    void AnalysisRulersButtonClicked();
    void AnalysisUniformIDsButtonClicked();
    void AnalysisComputeButtonClicked();
    void AnalysisLinksButtonClicked();
    void AnalysisUncertaintyButtonClicked();
    void AnalysisPathsButtonClicked();
    void AnalysisExportButtonClicked();

private:
    Ui::QvisTrackingDialog *ui;
};

#endif // QVISTRACKINGDIALOG_H
