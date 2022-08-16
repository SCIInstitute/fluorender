#ifndef QVISMESHTRANSFORMDIALOG_H
#define QVISMESHTRANSFORMDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisMeshTransformDialog;
}

class QvisMeshTransformDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisMeshTransformDialog(QWidget *parent = nullptr);
    ~QvisMeshTransformDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void TranslateXChanged(double value);
    void TranslateYChanged(double value);
    void TranslateZChanged(double value);

    void RotateXChanged(double value);
    void RotateYChanged(double value);
    void RotateZChanged(double value);

    void ScaleXChanged(double value);
    void ScaleYChanged(double value);
    void ScaleZChanged(double value);

private:
    Ui::QvisMeshTransformDialog *ui;
};

#endif // QVISMESHTRANSFORMDIALOG_H
