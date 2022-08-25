#include "QvisAnimationController.h"
#include "ui_QvisAnimationController.h"

#include <QThread>

#include <chrono>
#include <thread>

// QvisAnimationWorker is a threaded worker for animating through a dataset.
// It is threaded so that the UI can continue to be active. Namely the user
// needs to be able to stop the animation.
QvisAnimationWorker::QvisAnimationWorker()
{
}

void QvisAnimationWorker::process()
{
    // DO TO - add the needed code for signalling the next frame.

    // The animation can be quick so add a pause between frames.
    std::this_thread::sleep_for(std::chrono::milliseconds(mAnimationPause));

    emit finished();
}

QvisAnimationController::QvisAnimationController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QvisAnimationController)
{
    ui->setupUi(this);
}

QvisAnimationController::~QvisAnimationController()
{
    delete ui;
}

QvisAnimationController::AnimationSliderMode QvisAnimationController::getAnimationSliderMode() const
{
    return mAnimationSliderMode;
}

void QvisAnimationController::setAnimationSliderMode(AnimationSliderMode value)
{
    mAnimationSliderMode = value;
}

int QvisAnimationController::getAnimationPause() const
{
    return mAnimationPause;
}

void QvisAnimationController::setAnimationPause(int value)
{
    mAnimationPause = value;
}


// Animation
void QvisAnimationController::AnimationIndexSliderReleased()
{
    if(mAnimationSliderMode == SingleStep)
    {
        mAnimationStop = false;
        mAnimationContinuous = false;
        mAnimationStep = 0;

        Animation();
    }
}

void QvisAnimationController::AnimationSliderModeChanged(int value)
{
    mAnimationSliderMode = AnimationSliderMode(value);
}

void QvisAnimationController::AnimationPauseValueChanged(int value)
{
    mAnimationPause = value;
}

void QvisAnimationController::AnimationIndexSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->AnimationIndexSpinBox);
    ui->AnimationIndexSpinBox->setValue(value);
    ui->AnimationIndexSpinBox->setFocus();

    if(mAnimationSliderMode == Continuous)
    {
        mAnimationStop = false;
        mAnimationContinuous = false;
        mAnimationStep = 0;

        Animation();
    }
}

void QvisAnimationController::AnimationIndexValueChanged(int value)
{
    const QSignalBlocker blocker(ui->AnimationIndexSlider);
    ui->AnimationIndexSlider->setValue(value);

    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = 0;

    Animation();
}
void QvisAnimationController::AnimationReverseSingleStepClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = -1;

    Animation();
}

void QvisAnimationController::AnimationReverseClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = true;
    mAnimationStep = -1;

    Animation();
}

void QvisAnimationController::AnimationStopClicked()
{
    mAnimationStop = true;
    mAnimationContinuous = false;
    mAnimationStep = 0;
}

void QvisAnimationController::AnimationPlayClicked()
{
    mAnimationStop = false;
    mAnimationContinuous = true;
    mAnimationStep = 1;

    Animation();
}

void QvisAnimationController::AnimationForwardSingleStepClicked()
{

    mAnimationStop = false;
    mAnimationContinuous = false;
    mAnimationStep = 1;

    Animation();
}

void QvisAnimationController::Animation()
{
    // First update the counter.
    if(mAnimationStep)
    {
        int value = ui->AnimationIndexSlider->value() + mAnimationStep;

        if(ui->AnimationIndexSpinBox->minimum() <= value &&
           value <= ui->AnimationIndexSpinBox->maximum())
        {
            const QSignalBlocker blocker0(ui->AnimationIndexSpinBox);
            ui->AnimationIndexSpinBox->setValue(value);

            const QSignalBlocker blocker1(ui->AnimationIndexSlider);
            ui->AnimationIndexSlider->setValue(value);
        }
        else
        {
            ui->VCRWidget->StopClicked();
        }
    }

    // If the animation state is not stopped create a thread and worker.
    // do the animation A thread is necessary so to allow the continued
    // processing of UI events. Namely allowing the user to stop the animation.
    if(mAnimationStop == false)
    {
        QvisAnimationWorker* worker = new QvisAnimationWorker();

        worker->mAnimationPause = mAnimationPause;

        QThread* thread = new QThread();

        // Assign the worker to the thread.
        worker->moveToThread(thread);

        // If running continuously animate the next frame otherwise stop.
        if(mAnimationContinuous)
            connect(worker, SIGNAL(finished()), this, SLOT(Animation()));
        else
            mAnimationStop = true;

        // When the thread is started the worker process begins.
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        // When the worker is finished the thread quits.
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        // Take care of cleaning up when finished too
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        // Start the thread.
        thread->start();
    }
}

