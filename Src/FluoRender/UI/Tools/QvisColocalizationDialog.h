#ifndef QVISCOLOCALIZATIONDIALOG_H
#define QVISCOLOCALIZATIONDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisColocalizationDialog;
}

class QvisColocalizationDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisColocalizationDialog(QWidget *parent = nullptr);
    ~QvisColocalizationDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ThresholdLogicalANDButtonClicked();
    void MinimumValueButtonClicked();
    void ProductButtonClicked();
    void RatioToggled(bool checked);
    void IntensityWeightedToggled(bool checked);
    void PhysicalSizeToggled(bool checked);
    void ColormapToggled(bool checked);
    void SelectedStructuresOnlyToggled(bool checked);
    void AutoUpdateToggled(bool checked);
    void HoldHistoryToggled(bool checked);
    void HoldHistoryValueChanged(int value);
    void ClearHistoryButtonClicked();
    void AnalyzeButtonClicked();

private:
    Ui::QvisColocalizationDialog *ui;
};

#endif // QVISCOLOCALIZATIONDIALOG_H
