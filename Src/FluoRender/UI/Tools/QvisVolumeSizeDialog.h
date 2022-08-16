#ifndef QVISVOLUMESIZEDIALOG_H
#define QVISVOLUMESIZEDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisVolumeSizeDialog;
}

class QvisVolumeSizeDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisVolumeSizeDialog(QWidget *parent = nullptr);
    ~QvisVolumeSizeDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void MinimumValueChanged(int value);
    void MaximumValueChanged(int value);
    void MaximumToggled(bool checked);
    void SelectedStructuresOnlyToggled(bool checked);
    void AnalyzeButtonClicked();
private:
    Ui::QvisVolumeSizeDialog *ui;
};

#endif // QVISVOLUMESIZEDIALOG_H
