#include "QvisCalculationDialog.h"
#include "ui_QvisCalculationDialog.h"

QvisCalculationDialog::QvisCalculationDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisCalculationDialog)
{
    ui->setupUi(this);
}

QvisCalculationDialog::~QvisCalculationDialog()
{
    delete ui;
}

void QvisCalculationDialog::updateWindow(bool doAll)
{

}

void QvisCalculationDialog::OperandALoadButtonClicked()
{

}

void QvisCalculationDialog::OperandATextEditFinished()
{

}

void QvisCalculationDialog::OperandBLoadButtonClicked()
{

}

void QvisCalculationDialog::OperandBTextEditFinished()
{

}
void QvisCalculationDialog::ConsolidateVoxelsButtonClicked()
{

}

void QvisCalculationDialog::CombineGroupButtonClicked()
{

}

void QvisCalculationDialog::AddButtonClicked()
{

}

void QvisCalculationDialog::SubtractButtonClicked()
{

}

void QvisCalculationDialog::DivideButtonClicked()
{

}

void QvisCalculationDialog::ColocalizeButtonClicked()
{

}
