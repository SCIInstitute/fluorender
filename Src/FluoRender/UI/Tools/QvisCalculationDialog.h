#ifndef QVISCALCULATIONDIALOG_H
#define QVISCALCULATIONDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisCalculationDialog;
}

class QvisCalculationDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisCalculationDialog(QWidget *parent = nullptr);
    ~QvisCalculationDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void OperandALoadButtonClicked();
    void OperandATextEditFinished();
    void OperandBLoadButtonClicked();
    void OperandBTextEditFinished();
    void ConsolidateVoxelsButtonClicked();
    void CombineGroupButtonClicked();
    void AddButtonClicked();
    void SubtractButtonClicked();
    void DivideButtonClicked();
    void ColocalizeButtonClicked();

private:
    Ui::QvisCalculationDialog *ui;
};

#endif // QVISCALCULATIONDIALOG_H
