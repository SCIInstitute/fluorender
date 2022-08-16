#ifndef QVISCLIPDIALOG_H
#define QVISCLIPDIALOG_H

#include "QvisDialogBase.h"

class QCheckBox;
class QSpinBox;
class QvisRangeSlider;

namespace Ui {
class QvisClipDialog;
}

class QvisClipDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisClipDialog(QWidget *parent = nullptr);
    ~QvisClipDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    // Main buttons
    void MakeDefaultClicked();
    void ResetAllClicked();

    // Clipping plane
    // Clipping plane display
    void PlaneDisplayModeChanged(int index);
    void AlwaysDisplayToggled(bool checked);
    void ApplytoAllToggled(bool checked);
    void ResetPlanesClicked();
    // X plane
    void XPlaneSliderLowerChanged(int value);
    void XPlaneSliderUpperChanged(int value);
    void XPlaneStartValueChanged(int value);
    void XPlaneEndValueChanged(int value);
    void XPlaneSlabValueChanged(int value);
    void XPlaneSlabLockToggled(bool checked);
    // Y plane
    void YPlaneSliderLowerChanged(int value);
    void YPlaneSliderUpperChanged(int value);
    void YPlaneStartValueChanged(int value);
    void YPlaneEndValueChanged(int value);
    void YPlaneSlabValueChanged(int value);
    void YPlaneSlabLockToggled(bool checked);
    // Y plane
    void ZPlaneSliderLowerChanged(int value);
    void ZPlaneSliderUpperChanged(int value);
    void ZPlaneStartValueChanged(int value);
    void ZPlaneEndValueChanged(int value);
    void ZPlaneSlabValueChanged(int value);
    void ZPlaneSlabLockToggled(bool checked);
    // Clipping plane orientation
    void AlignToViewClicked();
    void ResetOrientationClicked();
    void XPlaneOrientationValueChanged(int value);
    void XPlaneOrientationSliderChanged(int value);
    void YPlaneOrientationValueChanged(int value);
    void YPlaneOrientationSliderChanged(int value);
    void ZPlaneOrientationValueChanged(int value);
    void ZPlaneOrientationSliderChanged(int value);

private:
    Ui::QvisClipDialog *ui;

    enum PLANE { X = 0, Y = 1, Z = 2 };

    // All planes
    void PlaneSliderLowerChanged(int value);
    void PlaneSliderUpperChanged(int value);
    void PlaneStartValueChanged(int value);
    void PlaneEndValueChanged(int value);
    void PlaneSlabValueChanged(int value);
    void PlaneSlabLockToggled(bool checked);

    bool mXPlaneSlabFlag{true};
    bool mYPlaneSlabFlag{true};
    bool mZPlaneSlabFlag{true};

    void setCurrentPlane( PLANE plane );

    QvisRangeSlider *mPlaneSlider{nullptr};
    QSpinBox        *mPlaneStartSpinBox{nullptr};
    QSpinBox        *mPlaneEndSpinBox{nullptr};
    QSpinBox        *mPlaneSlabSpinBox{nullptr};
    QCheckBox       *mPlaneSlabLockCheckBox{nullptr};

    bool *mPlaneSlabFlag{nullptr};

    int mXPlaneDefaultStart{0};
    int mXPlaneDefaultEnd{99};
    int mYPlaneDefaultStart{0};
    int mYPlaneDefaultEnd{99};
    int mZPlaneDefaultStart{0};
    int mZPlaneDefaultEnd{99};

    int mXPlaneDefaultOrientation{0};
    int mYPlaneDefaultOrientation{0};
    int mZPlaneDefaultOrientation{0};
};

#endif // QVISCLIPDIALOG_H
