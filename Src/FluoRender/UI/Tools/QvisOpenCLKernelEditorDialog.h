#ifndef QVISOPENCLKERNELEDITORDIALOG_H
#define QVISOPENCLKERNELEDITORDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisOpenCLKernelEditorDialog;
}

class QvisOpenCLKernelEditorDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisOpenCLKernelEditorDialog(QWidget *parent = nullptr);
    ~QvisOpenCLKernelEditorDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void KernelFileTextEditFinished();
    void BrowseButtonClicked();
    void SaveButtonClicked();
    void SaveAsButtonClicked();
    void KernelTableWidgetCellChanged(int row, int column);
    void RunButtonClicked();
    void IterationsValueChanged(int value);

private:
    Ui::QvisOpenCLKernelEditorDialog *ui;
};

#endif // QVISOPENCLKERNELEDITORDIALOG_H
