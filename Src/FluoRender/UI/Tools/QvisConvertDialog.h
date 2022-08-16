#ifndef QVISCONVERTDIALOG_H
#define QVISCONVERTDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisConvertDialog;
}

class QvisConvertDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisConvertDialog(QWidget *parent = nullptr);
    ~QvisConvertDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ThresholdSliderChanged(int value);
    void ThresholdValueChanged(double value);
    void DownSampleXYSliderChanged(int value);
    void DownSampleXYValueChanged(int value);
    void DownSampleZSliderChanged(int value);
    void DownSampleZValueChanged(int value);

    void UseTransferFunctionToggled(bool value);
    void SelectedOnlyToggled(bool value);
    void WeldVerticesToggled(bool value);

    void ConvertClicked();

private:
    Ui::QvisConvertDialog *ui;
};

#endif // QVISCONVERTDIALOG_H
