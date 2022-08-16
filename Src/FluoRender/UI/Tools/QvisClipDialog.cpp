#include "QvisClipDialog.h"
#include "ui_QvisClipDialog.h"

QvisClipDialog::QvisClipDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisClipDialog)
{
    ui->setupUi(this);

    ui->XPlaneStartLine->setHidden(true);
    ui->YPlaneStartLine->setHidden(true);
    ui->ZPlaneStartLine->setHidden(true);

    ui->XPlaneEndLine->setHidden(true);
    ui->YPlaneEndLine->setHidden(true);
    ui->ZPlaneEndLine->setHidden(true);

    ui->XPlaneSlider->SetGrooveColor(QColor(Qt::darkMagenta));
    ui->YPlaneSlider->SetGrooveColor(QColor(Qt::darkYellow));
    ui->ZPlaneSlider->SetGrooveColor(QColor(Qt::darkCyan));

    ui->XPlaneSlider->SetLowerHandleColor(QColor(Qt::red));
    ui->XPlaneSlider->SetUpperHandleColor(QColor(Qt::magenta));
    ui->YPlaneSlider->SetLowerHandleColor(QColor(Qt::green));
    ui->YPlaneSlider->SetUpperHandleColor(QColor(Qt::yellow));
    ui->ZPlaneSlider->SetLowerHandleColor(QColor(Qt::blue));
    ui->ZPlaneSlider->SetUpperHandleColor(QColor(Qt::cyan));

//    ui->XPlaneSlider->setStyleSheet("QSlider::groove:horizontal {background: blue;}");

//    ui->XPlaneStartSpinBox->setStyleSheet("QSpinBox { background-color: red; }");
//    ui->XPlaneEndSpinBox->setStyleSheet("QSpinBox { background-color: magenta; }");
//    ui->YPlaneStartSpinBox->setStyleSheet("QSpinBox { background-color: lightgreen; }");
//    ui->YPlaneEndSpinBox->setStyleSheet("QSpinBox { background-color: yellow; }");
//    ui->ZPlaneStartSpinBox->setStyleSheet("QSpinBox { background-color: lightblue; }");
//    ui->ZPlaneEndSpinBox->setStyleSheet("QSpinBox { background-color: cyan; }");

//    ui->XPlaneOrientationSpinBox->setStyleSheet("QSpinBox { background-color: red; }");
//    ui->YPlaneOrientationSpinBox->setStyleSheet("QSpinBox { background-color: green; }");
//    ui->ZPlaneOrientationSpinBox->setStyleSheet("QSpinBox { background-color: lightblue; }");

    // Changing the groove color nukes the handles - qt bug.
//    ui->XPlaneOrientationSlider->setStyleSheet(//"QSlider::groove:horizontal {border: 1px solid #999999; height: 3px; border-radius: 1px; background: red;}"
//                                               "QSlider::handle:horizontal {background: red; border: 3px solid #5c5c5c; width: 10px; border-radius: 3px; }");
    //ui->YPlaneOrientationSlider->setStyleSheet("QSlider::groove:horizontal {background: green;}");
    //ui->ZPlaneOrientationSlider->setStyleSheet("QSlider::groove:horizontal {background: blue;}");
}

QvisClipDialog::~QvisClipDialog()
{
    delete ui;
}

void QvisClipDialog::updateWindow(bool doAll)
{

}

// Main buttons
void QvisClipDialog::MakeDefaultClicked()
{
    // Planes
    mXPlaneDefaultStart = ui->XPlaneStartSpinBox->value();
    mXPlaneDefaultEnd   = ui->XPlaneEndSpinBox->value();
    mYPlaneDefaultStart = ui->YPlaneStartSpinBox->value();
    mYPlaneDefaultEnd   = ui->YPlaneEndSpinBox->value();
    mZPlaneDefaultStart = ui->ZPlaneStartSpinBox->value();
    mZPlaneDefaultEnd   = ui->ZPlaneEndSpinBox->value();

    // Orientation
    mXPlaneDefaultOrientation = ui->XPlaneOrientationSpinBox->value();
    mYPlaneDefaultOrientation = ui->YPlaneOrientationSpinBox->value();
    mZPlaneDefaultOrientation = ui->ZPlaneOrientationSpinBox->value();
}

void QvisClipDialog::ResetAllClicked()
{
    notify = false;

    // Planes
    XPlaneStartValueChanged(mXPlaneDefaultStart);
    XPlaneEndValueChanged  (mXPlaneDefaultEnd);
    YPlaneStartValueChanged(mYPlaneDefaultStart);
    YPlaneEndValueChanged  (mYPlaneDefaultEnd);
    ZPlaneStartValueChanged(mZPlaneDefaultStart);
    ZPlaneEndValueChanged  (mZPlaneDefaultEnd);

    // Orientation
    XPlaneOrientationValueChanged(mXPlaneDefaultOrientation);
    YPlaneOrientationValueChanged(mYPlaneDefaultOrientation);
    ZPlaneOrientationValueChanged(mZPlaneDefaultOrientation);

    notify = true;
//    sendNotification();
}

// Clipping plane
// Clipping plane display
void QvisClipDialog::PlaneDisplayModeChanged(int index)
{

//    sendNotification();
}

void QvisClipDialog::AlwaysDisplayToggled(bool checked)
{

//    sendNotification();
}

void QvisClipDialog::ApplytoAllToggled(bool checked)
{

//    sendNotification();
}

void QvisClipDialog::ResetPlanesClicked()
{
    notify = false;

    XPlaneStartValueChanged(mXPlaneDefaultStart);
    XPlaneEndValueChanged  (mXPlaneDefaultEnd);
    YPlaneStartValueChanged(mYPlaneDefaultStart);
    YPlaneEndValueChanged  (mYPlaneDefaultEnd);
    ZPlaneStartValueChanged(mZPlaneDefaultStart);
    ZPlaneEndValueChanged  (mZPlaneDefaultEnd);

    notify = true;
//    sendNotification();
}

// X plane
void QvisClipDialog::XPlaneSliderLowerChanged(int value)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneStartValueChanged(value);
}

void QvisClipDialog::XPlaneStartValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneStartValueChanged(value);
}

void QvisClipDialog::XPlaneSliderUpperChanged(int value)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneEndValueChanged(value);
}

void QvisClipDialog::XPlaneEndValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneEndValueChanged(value);
}

void QvisClipDialog::XPlaneSlabValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneSlabValueChanged(value);
}

void QvisClipDialog::XPlaneSlabLockToggled(bool checked)
{
    setCurrentPlane( QvisClipDialog::X );
    this->PlaneSlabLockToggled(checked);
}

// Y plane
void QvisClipDialog::YPlaneStartValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Y );
    this->PlaneStartValueChanged(value);
}

void QvisClipDialog::YPlaneSliderLowerChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Y );
    this->PlaneSliderLowerChanged(value);
}

void QvisClipDialog::YPlaneEndValueChanged(int value)
{    setCurrentPlane( QvisClipDialog::Y );
     this->PlaneEndValueChanged(value);
}

void QvisClipDialog::YPlaneSliderUpperChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Y );
    this->PlaneSliderUpperChanged(value);
}

void QvisClipDialog::YPlaneSlabValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Y );
    this->PlaneSlabValueChanged(value);
}

void QvisClipDialog::YPlaneSlabLockToggled(bool checked)
{
    setCurrentPlane( QvisClipDialog::Y );
    this->PlaneSlabLockToggled(checked);
}

// Z plane
void QvisClipDialog::ZPlaneStartValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneStartValueChanged(value);
}

void QvisClipDialog::ZPlaneSliderLowerChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneSliderLowerChanged(value);
}

void QvisClipDialog::ZPlaneEndValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneEndValueChanged(value);
}

void QvisClipDialog::ZPlaneSliderUpperChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneSliderUpperChanged(value);
}

void QvisClipDialog::ZPlaneSlabValueChanged(int value)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneSlabValueChanged(value);
}

void QvisClipDialog::ZPlaneSlabLockToggled(bool checked)
{
    setCurrentPlane( QvisClipDialog::Z );
    this->PlaneSlabLockToggled(checked);
}

// All planes
void QvisClipDialog::setCurrentPlane( PLANE plane )
{
    if( plane == QvisClipDialog::X )
    {
        mPlaneSlider           = ui->XPlaneSlider;
        mPlaneStartSpinBox     = ui->XPlaneStartSpinBox;
        mPlaneEndSpinBox       = ui->XPlaneEndSpinBox;
        mPlaneSlabSpinBox      = ui->XPlaneSlabSpinBox;
        mPlaneSlabLockCheckBox = ui->XPlaneSlabLockCheckBox;

        mPlaneSlabFlag         = &mXPlaneSlabFlag;
    }
    else if( plane == QvisClipDialog::Y )
    {
        mPlaneSlider           = ui->YPlaneSlider;
        mPlaneStartSpinBox     = ui->YPlaneStartSpinBox;
        mPlaneEndSpinBox       = ui->YPlaneEndSpinBox;
        mPlaneSlabSpinBox      = ui->YPlaneSlabSpinBox;
        mPlaneSlabLockCheckBox = ui->YPlaneSlabLockCheckBox;

        mPlaneSlabFlag         = &mYPlaneSlabFlag;
    }
    else if( plane == QvisClipDialog::Z )
    {
        mPlaneSlider           = ui->ZPlaneSlider;
        mPlaneStartSpinBox     = ui->ZPlaneStartSpinBox;
        mPlaneEndSpinBox       = ui->ZPlaneEndSpinBox;
        mPlaneSlabSpinBox      = ui->ZPlaneSlabSpinBox;
        mPlaneSlabLockCheckBox = ui->ZPlaneSlabLockCheckBox;

        mPlaneSlabFlag         = &mZPlaneSlabFlag;
    }
}

void QvisClipDialog::PlaneSliderLowerChanged(int value)
{
    this->PlaneStartValueChanged(value);
}

void QvisClipDialog::PlaneStartValueChanged(int value)
{
    const QSignalBlocker blocker0(mPlaneSlider);
    const QSignalBlocker blocker1(mPlaneStartSpinBox);
    const QSignalBlocker blocker2(mPlaneEndSpinBox);
    const QSignalBlocker blocker3(mPlaneSlabSpinBox);

    // Slab width is constanst.
    if( mPlaneSlabLockCheckBox->isChecked() )
    {
        // Subtract one to account for being inclusive.
      int width = mPlaneSlabSpinBox->value() - 1;

      // If the start plus the width is greater than the maximum reset the start.
      if( value + width > mPlaneStartSpinBox->maximum())
      {
          value = mPlaneStartSpinBox->maximum() - width;
      }

      // Adjust the other values.
      mPlaneSlider->setLowerValue(value);
      mPlaneSlider->setUpperValue(value+width);
      mPlaneStartSpinBox->setValue(value);
      mPlaneEndSpinBox->setValue(value+width);
      mPlaneEndSpinBox->setFocus();
    }
    else
    {
        if(value > mPlaneEndSpinBox->value())
            value = mPlaneEndSpinBox->value();

        mPlaneSlider->setLowerValue(value);
        mPlaneStartSpinBox->setValue(value);
        mPlaneStartSpinBox->setFocus();

        // Add one to account for being inclusive.
        int width = (mPlaneEndSpinBox->value() -
                     mPlaneStartSpinBox->value() + 1);

        mPlaneSlabSpinBox->setValue(width);
    }

//    sendNotification();
}

void QvisClipDialog::PlaneSliderUpperChanged(int value)
{
    this->PlaneEndValueChanged( value );
}

void QvisClipDialog::PlaneEndValueChanged(int value)
{
    const QSignalBlocker blocker0(mPlaneSlider);
    const QSignalBlocker blocker1(mPlaneStartSpinBox);
    const QSignalBlocker blocker2(mPlaneEndSpinBox);
    const QSignalBlocker blocker3(mPlaneSlabSpinBox);

    // Slab width is constanst.
    if( mPlaneSlabLockCheckBox->isChecked() )
    {
      // Subtract one to account for being inclusive.
      int width = mPlaneSlabSpinBox->value() - 1;

      // If the end less the width is less than the minimum reset the end.
      if( value - width < mPlaneStartSpinBox->minimum())
      {
          value = mPlaneStartSpinBox->minimum() + width;
      }

      // Adjust the other values.
      mPlaneSlider->setLowerValue(value-width);
      mPlaneSlider->setUpperValue(value);
      mPlaneStartSpinBox->setValue(value-width);
      mPlaneEndSpinBox->setValue(value);
      mPlaneEndSpinBox->setFocus();
    }
    else
    {
        if(value < mPlaneStartSpinBox->value())
            value = mPlaneStartSpinBox->value();

        mPlaneSlider->setUpperValue(value);
        mPlaneEndSpinBox->setValue(value);
        mPlaneEndSpinBox->setFocus();

        // Add one to account for being inclusive.
        int width = (mPlaneEndSpinBox->value() -
                     mPlaneStartSpinBox->value() + 1);

        mPlaneSlabSpinBox->setValue(width);
    }

//    sendNotification();
}

void QvisClipDialog::PlaneSlabValueChanged(int value)
{
    const QSignalBlocker blocker0(mPlaneSlider);
    const QSignalBlocker blocker1(mPlaneStartSpinBox);
    const QSignalBlocker blocker2(mPlaneEndSpinBox);

    // Subtract one to account for being inclusive.
    value -= 1;

    // Get the current width
    int minimum = mPlaneStartSpinBox->minimum();
    int maximum = mPlaneStartSpinBox->maximum();

    int start = mPlaneStartSpinBox->value();
    int end   = mPlaneEndSpinBox->value();

    int width = end - start;

    int offset = (width < value ) ? +1 : -1;

   bool first = true;

    while( 1 )
    {
        if(!first || (first && *mPlaneSlabFlag))
        {
            start -= offset;

            if(start < minimum)
                start = minimum;

            if(start > maximum)
               start = maximum;

            if(end - start == value)
               break;
        }

        end += offset;

        if(end < minimum)
            end = minimum;

        if(start > maximum)
            end = maximum;

        if(end - start == value)
            break;
    }

    *mPlaneSlabFlag = *mPlaneSlabFlag ? false : true;

    mPlaneSlider->setLowerValue(start);
    mPlaneSlider->setUpperValue(end);
    mPlaneStartSpinBox->setValue(start);
    mPlaneEndSpinBox->setValue(end);

//    sendNotification();
}

void QvisClipDialog::PlaneSlabLockToggled(bool checked)
{

}

// Clipping plane orientation
void QvisClipDialog::AlignToViewClicked()
{

//    sendNotification();
}

void QvisClipDialog::ResetOrientationClicked()
{
    notify = false;

    XPlaneOrientationValueChanged(mXPlaneDefaultOrientation);
    YPlaneOrientationValueChanged(mYPlaneDefaultOrientation);
    ZPlaneOrientationValueChanged(mZPlaneDefaultOrientation);

    notify = true;
//    sendNotification();
}

void QvisClipDialog::XPlaneOrientationValueChanged(int value)
{
    const QSignalBlocker blocker(ui->XPlaneOrientationSlider);
    ui->XPlaneOrientationSlider->setValue(value);

//    sendNotification();
}

void QvisClipDialog::XPlaneOrientationSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->XPlaneOrientationSpinBox);
    ui->XPlaneOrientationSpinBox->setValue(value);
    ui->XPlaneOrientationSpinBox->setFocus();

//    sendNotification();
}

void QvisClipDialog::YPlaneOrientationValueChanged(int value)
{
    const QSignalBlocker blocker(ui->YPlaneOrientationSlider);
    ui->YPlaneOrientationSlider->setValue(value);

//    sendNotification();
}

void QvisClipDialog::YPlaneOrientationSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->YPlaneOrientationSpinBox);
    ui->YPlaneOrientationSpinBox->setValue(value);
    ui->YPlaneOrientationSpinBox->setFocus();

//    sendNotification();
}

void QvisClipDialog::ZPlaneOrientationValueChanged(int value)
{
    const QSignalBlocker blocker(ui->ZPlaneOrientationSlider);
    ui->ZPlaneOrientationSlider->setValue(value);

//    sendNotification();
}

void QvisClipDialog::ZPlaneOrientationSliderChanged(int value)
{
    const QSignalBlocker blocker(ui->ZPlaneOrientationSpinBox);
    ui->ZPlaneOrientationSpinBox->setValue(value);
    ui->ZPlaneOrientationSpinBox->setFocus();

//    sendNotification();
}
