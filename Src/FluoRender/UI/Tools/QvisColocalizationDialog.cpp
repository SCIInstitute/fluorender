#include "QvisColocalizationDialog.h"
#include "ui_QvisColocalizationDialog.h"

QvisColocalizationDialog::QvisColocalizationDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisColocalizationDialog)
{
    ui->setupUi(this);
}

QvisColocalizationDialog::~QvisColocalizationDialog()
{
    delete ui;
}

void QvisColocalizationDialog::updateWindow(bool doAll)
{

}

void QvisColocalizationDialog::ThresholdLogicalANDButtonClicked()
{

}

void QvisColocalizationDialog::MinimumValueButtonClicked()
{

}

void QvisColocalizationDialog::ProductButtonClicked()
{

}

void QvisColocalizationDialog::RatioToggled(bool checked)
{

}

void QvisColocalizationDialog::IntensityWeightedToggled(bool checked)
{

}

void QvisColocalizationDialog::PhysicalSizeToggled(bool checked)
{

}

void QvisColocalizationDialog::ColormapToggled(bool checked)
{

}

void QvisColocalizationDialog::SelectedStructuresOnlyToggled(bool checked)
{

}

void QvisColocalizationDialog::AutoUpdateToggled(bool checked)
{

}

void QvisColocalizationDialog::HoldHistoryToggled(bool checked)
{

}

void QvisColocalizationDialog::HoldHistoryValueChanged(int value)
{

}

void QvisColocalizationDialog::ClearHistoryButtonClicked()
{
    while( ui->SelectionTableWidget->rowCount() )
      ui->SelectionTableWidget->removeRow( ui->SelectionTableWidget->rowCount() - 1 );
}

void QvisColocalizationDialog::AnalyzeButtonClicked()
{

}

