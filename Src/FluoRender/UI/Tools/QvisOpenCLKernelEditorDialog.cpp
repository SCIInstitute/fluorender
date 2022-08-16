#include "QvisOpenCLKernelEditorDialog.h"
#include "ui_QvisOpenCLKernelEditorDialog.h"

#include <QFileDialog>

#include <iostream>

QvisOpenCLKernelEditorDialog::QvisOpenCLKernelEditorDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisOpenCLKernelEditorDialog)
{
    ui->setupUi(this);
}

QvisOpenCLKernelEditorDialog::~QvisOpenCLKernelEditorDialog()
{
    delete ui;
}

void QvisOpenCLKernelEditorDialog::updateWindow(bool doAll)
{

}

void QvisOpenCLKernelEditorDialog::KernelFileTextEditFinished()
{
    ui->KernelFileTextEdit->text();
}

void QvisOpenCLKernelEditorDialog::BrowseButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    //dialog.setDirectory();

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList fileNames = dialog.selectedFiles();

        if( fileNames.size() == 1)
        {
            std::cerr << fileNames[0].toStdString() << std::endl;
            const QSignalBlocker blocker(ui->KernelFileTextEdit);
            ui->KernelFileTextEdit->setText(fileNames[0]);
            // Update attributes.
        }
    }

    this->raise();
}

void QvisOpenCLKernelEditorDialog::SaveButtonClicked()
{

}

void QvisOpenCLKernelEditorDialog::SaveAsButtonClicked()
{

}

void QvisOpenCLKernelEditorDialog::KernelTableWidgetCellChanged(int row, int column)
{

}

void QvisOpenCLKernelEditorDialog::RunButtonClicked()
{

}

void QvisOpenCLKernelEditorDialog::IterationsValueChanged(int value)
{

}

