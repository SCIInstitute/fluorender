#include "QvisOpenGLWidget.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include <iostream>

QvisOpenGLWidget::QvisOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);  // For Key presses
    setMouseTracking(true);           // For tracking the mouse anytime it is in the window.
}

QvisOpenGLWidget::~QvisOpenGLWidget()
{
}


void QvisOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    QOpenGLWidget::initializeGL();

    emit initializeWindow(this->objectName());
}

void QvisOpenGLWidget::paintGL()
{
    //QOpenGLWidget::paintGL();

    if(!this->isVisible())
        return;

    emit paintWindow(this->objectName());
}

void QvisOpenGLWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    emit resizeWindow(this->objectName(), w, h);
}

void QvisOpenGLWidget::enterEvent(QEnterEvent * event)
{
    QOpenGLWidget::enterEvent(event);

    emit activeWindow(this->objectName());
}

void QvisOpenGLWidget::leaveEvent(QEvent * event)
{
    QOpenGLWidget::leaveEvent(event);

    std::cerr << __FUNCTION__ << "  " << __LINE__ << std::endl;
    emit activeWindow(QString());
}

void QvisOpenGLWidget::mousePressEvent(QMouseEvent* aEvent)
{
    QOpenGLWidget::mousePressEvent(aEvent);

    emit activeWindow(this->objectName());

    if(aEvent->buttons() & Qt::LeftButton)
    {
        mMouseDown = true;

        this->update();

        emit mousePress(aEvent->position());
    }
    else if(aEvent->buttons() & Qt::RightButton)
    {
        emit customContextMenuRequested(QPoint(aEvent->globalPosition().x(),
                                               aEvent->globalPosition().y()));
    }
}

void QvisOpenGLWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
    QOpenGLWidget::mouseMoveEvent(aEvent);

    if((aEvent->buttons() & Qt::LeftButton && mAltDown) ||
       (aEvent->buttons() & Qt::MiddleButton))
    {
        // Scale
    }
    else if(aEvent->buttons() & Qt::LeftButton && mShiftDown)
    {
        // Pan
    }
    else if(aEvent->buttons() & Qt::LeftButton)
    {
        // Rotate
    }

    if(mMouseDown)
    {
        emit mouseMove(aEvent->position());

        this->update();
    }
}

void QvisOpenGLWidget::mouseReleaseEvent(QMouseEvent* aEvent)
{
    QOpenGLWidget::mouseReleaseEvent(aEvent);

    mMouseDown = false;

    this->update();

    emit mouseRelease(aEvent->position());
}

void QvisOpenGLWidget::keyPressEvent(QKeyEvent* aEvent)
{
    QOpenGLWidget::keyPressEvent(aEvent);

    if(aEvent->key() & Qt::Key_Shift) // For Pan
    {
        mShiftDown = true;
    }
    else if(aEvent->key() & Qt::Key_Control) // For context menu
    {
        mCtrlDown = true;
    }
    else if(aEvent->key() & Qt::Key_Alt) // For scale
    {
        mAltDown = true;
    }
    else if(aEvent->key() & Qt::Key_Meta)
    {
        mMetaDown = true;
    }

//    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << aEvent->key() << "  " << aEvent->text().toStdString() << std::endl;
}

void QvisOpenGLWidget::keyReleaseEvent(QKeyEvent* aEvent)
{
    QOpenGLWidget::keyReleaseEvent(aEvent);

    if(aEvent->key() & Qt::Key_Shift) // For Pan
    {
        mShiftDown = false;
    }
    else if(aEvent->key() & Qt::Key_Control) // For context menu
    {
        mCtrlDown = false;
    }
    else if(aEvent->key() & Qt::Key_Alt) // For scale
    {
        mAltDown = false;
    }
    else if(aEvent->key() & Qt::Key_Meta)
    {
        mMetaDown = false;
    }

    //    std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << aEvent->key() << "  " << aEvent->text().toStdString() << std::endl;
}
