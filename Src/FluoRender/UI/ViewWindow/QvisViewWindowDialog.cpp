#include "QvisViewWindowDialog.h"
#include "ui_QvisViewWindowDialog.h"

#include <QKeyEvent>

#include "QvisViewWindow.h"
#include "QvisOpenGLWidget.h"

#include <iostream>

QvisViewWindowDialog::QvisViewWindowDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisViewWindowDialog)
{
    ui->setupUi(this);

//    QObject::connect(this, SIGNAL(destroyed()),   this, SLOT(ViewWindowDialogDestroyed()));
//    QObject::connect(this, SIGNAL(finished(int)), this, SLOT(Finished(int)));
}

QvisViewWindowDialog::~QvisViewWindowDialog()
{
    delete ui;
}

void QvisViewWindowDialog::updateWindow(bool doAll)
{

}

// Used when the ADS is NOT avabile as a dialog for the ViewWindow.
void QvisViewWindowDialog::setWidget(QvisViewWindow *viewWindow)
{
    if(viewWindow)
    {
        viewWindow->setParent(this);
        viewWindow->setViewWindowDialog(this);

        this->setWindowTitle(viewWindow->windowTitle());

        ui->MainLayout->addWidget(viewWindow);
    }
}

void QvisViewWindowDialog::removeWidget(QvisViewWindow *viewWindow)
{
    ui->MainLayout->removeWidget(viewWindow);

    viewWindow->setParent(nullptr);
    viewWindow->setViewWindowDialog(nullptr);
}

// Used as a full screen dialog for a SINGLE openGLWindow.
void QvisViewWindowDialog::setWidget(QvisOpenGLWidget *openGLWindow)
{
    // When setting an openGlWindow it will be used for a temporary full screen window only.
    if(openGLWindow)
    {
        fullScreen = true;

        // No border for the layout.
        ui->MainLayout->setContentsMargins(0,0,0,0);
        ui->MainLayout->addWidget(openGLWindow);
    }
}

void QvisViewWindowDialog::removeWidget(QvisOpenGLWidget *openGLWindow)
{
    ui->MainLayout->removeWidget(openGLWindow);
}

//void QvisViewWindowDialog::ViewWindowDialogDestroyed()
//{
//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "'  " << std::endl;
//    emit viewWindowDialogDestroyed(this->windowTitle());
//}

//void QvisViewWindowDialog::Finished(int result)
//{
//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "'  " << result << std::endl;
//    //emit viewWindowDialogDestroyed(this->windowTitle());
//}

void QvisViewWindowDialog::changeEvent(QEvent *aEvent)
{
    //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  '" << this->windowTitle().toStdString() << "'  " << aEvent->type() << "  " << windowState() << std::endl;

    QvisDialogBase::changeEvent(aEvent);

    // If in full screen mode and the window state is no longer full screen
    // or if the window is no long the active window then emit a signal for
    // the parent to restore the openGLWindow then delete this dialog.
    if(fullScreen &&
       ((aEvent->type() == QEvent::WindowStateChange && !(this->windowState() & Qt::WindowFullScreen)) ||
        (aEvent->type() == QEvent::ActivationChange  && !this->isActiveWindow())))
        emit exitFullScreen();
}

void QvisViewWindowDialog::closeEvent(QCloseEvent *aEvent)
{
    // If in full screen mode not close as the openGLWindow will be destroyed.
    // Instead emit a signal for the parent to restore the openGLWindow then
    // delete this dialog.
    if(fullScreen)
        emit exitFullScreen();
    else
        QvisDialogBase::closeEvent(aEvent);
}

void QvisViewWindowDialog::keyPressEvent(QKeyEvent *aEvent)
{
    QvisDialogBase::keyPressEvent(aEvent);

    //std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << aEvent->key() << "  " << aEvent->text().toStdString() << std::endl;

    // If in full screen mode and the window state is no longer full screen
    // then emit a signal for the parent to restore the openGLWindow then
    // delete this dialog.
    if(fullScreen && aEvent->key() == Qt::Key_Escape)
        emit exitFullScreen();
}
