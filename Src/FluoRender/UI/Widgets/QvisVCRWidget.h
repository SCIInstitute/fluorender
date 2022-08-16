#ifndef QVISVCRWIDGET_H
#define QVISVCRWIDGET_H

#include <QWidget>

namespace Ui {
class QvisVCRWidget;
}

class QvisVCRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QvisVCRWidget(QWidget *parent = nullptr);
    ~QvisVCRWidget();

signals:
    void reverseSingleStep();
    void reverse();
    void stop();
    void play();
    void forwardSingleStep();

public slots:
    void ReverseSingleStepClicked();
    void ReverseClicked();
    void StopClicked();
    void PlayClicked();
    void ForwardSingleStepClicked();

private:
    Ui::QvisVCRWidget *ui;
};

#endif // QVISVCRWIDGET_H
