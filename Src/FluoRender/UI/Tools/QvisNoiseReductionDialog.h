#ifndef QVISNOISEREDUCTIONDIALOG_H
#define QVISNOISEREDUCTIONDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisNoiseReductionDialog;
}

class QvisNoiseReductionDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisNoiseReductionDialog(QWidget *parent = nullptr);
    ~QvisNoiseReductionDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ThresholdSliderChanged(int value);
    void ThresholdValueChanged(int value);
    void VoxelSizeSliderChanged(int value);
    void VoxelSizeValueChanged(double value);
    void SlectedRegionOnlyToggled(bool checked);
    void EnhanceSelectionToggled(bool checked);
    void EraseButtonClicked();
    void PreviewButtonClicked();

private:
    Ui::QvisNoiseReductionDialog *ui;
};

#endif // QVISNOISEREDUCTIONDIALOG_H
