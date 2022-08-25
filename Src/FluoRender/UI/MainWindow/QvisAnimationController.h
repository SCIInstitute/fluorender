#ifndef QVISANIMATIONCONTROLLER_H
#define QVISANIMATIONCONTROLLER_H

#include <QWidget>

namespace Ui {
class QvisAnimationController;
}

// QvisAnimationWorker is a threaded worker for animating through a dataset.
// It is threaded so that the UI can continue to be active. Namely the user
// needs to be able to stop the animation.
class QvisAnimationWorker : public QWidget
{
    Q_OBJECT

    friend class QvisAnimationController;

public:
    QvisAnimationWorker();

signals:
    void finished();

public slots:
    void process();

protected:
    int  mAnimationPause{250};
};

class QvisAnimationController : public QWidget
{
    Q_OBJECT

    enum AnimationSliderMode {
        SingleStep = 0x0000,
        Continuous = 0x0001,
    };

public:
    explicit QvisAnimationController(QWidget *parent = nullptr);
    ~QvisAnimationController();

    AnimationSliderMode getAnimationSliderMode() const;
    void setAnimationSliderMode(AnimationSliderMode value);

    int  getAnimationPause() const;
    void setAnimationPause(int value);

private slots:
    // Animation
    void AnimationReverseSingleStepClicked();
    void AnimationReverseClicked();
    void AnimationStopClicked();
    void AnimationPlayClicked();
    void AnimationForwardSingleStepClicked();
    void Animation();

    void AnimationSliderModeChanged(int);
    void AnimationPauseValueChanged(int);
    void AnimationIndexSliderReleased();
    void AnimationIndexSliderChanged(int);
    void AnimationIndexValueChanged(int);

private:
    Ui::QvisAnimationController *ui;

    // For animation
    AnimationSliderMode mAnimationSliderMode{SingleStep};
    int  mAnimationPause{250};

    bool mAnimationContinuous{false};
    bool mAnimationStop{true};
    int  mAnimationStep{0};
};

#endif // QVISANIMATIONCONTROLLER_H
