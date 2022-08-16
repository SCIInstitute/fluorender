#include "QvisVolumeSizeDialog.h"
#include "ui_QvisVolumeSizeDialog.h"

QvisVolumeSizeDialog::QvisVolumeSizeDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisVolumeSizeDialog)
{
    ui->setupUi(this);
}

QvisVolumeSizeDialog::~QvisVolumeSizeDialog()
{
    delete ui;
}

void QvisVolumeSizeDialog::updateWindow(bool doAll)
{

}

void QvisVolumeSizeDialog::MinimumValueChanged(int value)
{

}

void QvisVolumeSizeDialog::MaximumValueChanged(int value)
{

}

void QvisVolumeSizeDialog::MaximumToggled(bool checked)
{

}

void QvisVolumeSizeDialog::SelectedStructuresOnlyToggled(bool checked)
{

}

void QvisVolumeSizeDialog::AnalyzeButtonClicked()
{

}

