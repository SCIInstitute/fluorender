#include "QvisMeshTransformDialog.h"
#include "ui_QvisMeshTransformDialog.h"

QvisMeshTransformDialog::QvisMeshTransformDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisMeshTransformDialog)
{
    ui->setupUi(this);
}

QvisMeshTransformDialog::~QvisMeshTransformDialog()
{
    delete ui;
}

void QvisMeshTransformDialog::updateWindow(bool doAll)
{
}

void QvisMeshTransformDialog::TranslateXChanged(double value)
{

}

void QvisMeshTransformDialog::TranslateYChanged(double value)
{

}

void QvisMeshTransformDialog::TranslateZChanged(double value)
{

}

void QvisMeshTransformDialog::RotateXChanged(double value)
{

}

void QvisMeshTransformDialog::RotateYChanged(double value)
{

}

void QvisMeshTransformDialog::RotateZChanged(double value)
{

}

void QvisMeshTransformDialog::ScaleXChanged(double value)
{

}

void QvisMeshTransformDialog::ScaleYChanged(double value)
{

}

void QvisMeshTransformDialog::ScaleZChanged(double value)
{

}

