#include "QvisRangeSlider.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

#include <iostream>

QvisRangeSlider::QvisRangeSlider(QWidget* aParent)
    : QWidget(aParent)
{
    setMouseTracking(true);
}

QvisRangeSlider::QvisRangeSlider(Qt::Orientation orientation, SliderHandles type, QWidget* aParent)
    : QWidget(aParent),
      mOrientation(orientation),
      mType(type)
{
    setMouseTracking(true);
}

void QvisRangeSlider::paintEvent(QPaintEvent* aEvent)
{
    Q_UNUSED(aEvent);

    QPainter painter(this);

    // Check before painting in case the orientation changed.
    mHandleSize = mHandleWidth;

    if(mOrientation == Qt::Vertical)
    {
        mHandleWidth  = mHandleHeight;
        mHandleHeight = mHandleSize;

        painter.translate( 0, height() );
        painter.scale( 1, -1 );
    }

    QPen pen;
    pen.setStyle(Qt::SolidLine);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    // Background unselect grove - draws the full grove.
    QRectF backgroundRect;
    if(mOrientation == Qt::Horizontal)
        backgroundRect = QRectF(mMargin, (height() - mGrooveHeight) / 2, width() - mMargin * 2, mGrooveHeight);
    else
        backgroundRect = QRectF((width() - mGrooveHeight) / 2, mMargin, mGrooveHeight, height() - mMargin * 2);

    // Unselected grove pen and brush and paint.
    pen.setColor(mUnselectedGrooveBorderColor);
    pen.setWidthF(0.8);
    painter.setPen(pen);
    brush.setColor(mUnselectedGrooveColor);
    painter.setBrush(brush);
    painter.drawRoundedRect(backgroundRect, mGrooveHeight/2, mGrooveHeight/2);

    // Handle pen and brush.
    pen.setColor(mHandleBorderColor);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    // First value handle rect
    QRectF leftHandleRect = firstHandleRect();
    if(mType.testFlag(LeftHandle))
    {
        brush.setColor(mLeftHandleColor);
        painter.setBrush(brush);
        painter.drawRoundedRect(leftHandleRect, mHandleBorderRadius, mHandleBorderRadius);
    }

    // Second value handle rect
    QRectF rightHandleRect = secondHandleRect();
    if(mType.testFlag(RightHandle))
    {
        brush.setColor(mRightHandleColor);
        painter.setBrush(brush);
        painter.drawRoundedRect(rightHandleRect, mHandleBorderRadius, mHandleBorderRadius);
    }

    // Selected grove between the handles.
    QRectF selectedRect(backgroundRect);
    double offset = mType.testFlag(DoubleHandles) ? 0.5 : (mType.testFlag(LeftHandle) ? 1.0 : 0);

    if(mOrientation == Qt::Horizontal) {
        selectedRect.setLeft((mType.testFlag(LeftHandle) ? leftHandleRect.right() : leftHandleRect.left()) + offset);
        selectedRect.setRight((mType.testFlag(RightHandle) ? rightHandleRect.left() : rightHandleRect.right()) - offset);

    } else {
        selectedRect.setTop((mType.testFlag(LeftHandle) ? leftHandleRect.bottom() : leftHandleRect.top()) + offset);
        selectedRect.setBottom((mType.testFlag(RightHandle) ? rightHandleRect.top() : rightHandleRect.bottom()) - offset);
    }

    if(mType.testFlag(DoubleHandles) ||
            (mType.testFlag(LeftHandle)  && mLowerValue < mMaximum) ||
            (mType.testFlag(RightHandle) && mUpperValue > mMinimum))
    {
        // Selected grove between the handles color and brush.
        pen.setColor(mSelectedGrooveBorderColor);
        painter.setPen(pen);
        brush.setColor(mSelectedGrooveColor);
        painter.setBrush(brush);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.drawRect(selectedRect);
    }
}

QRectF QvisRangeSlider::firstHandleRect() const
{
    float percentage = float(mLowerValue - mMinimum) / float(mInterval);
    return handleRect(percentage * validLength() + mMargin);
}

QRectF QvisRangeSlider::secondHandleRect() const
{
    float percentage = float(mUpperValue - mMinimum) / float(mInterval);
    return handleRect(percentage * validLength() + mMargin + (mType.testFlag(LeftHandle) ? mHandleSize : 0));
}

QRectF QvisRangeSlider::handleRect(int aValue) const
{
    if(mOrientation == Qt::Horizontal)
        return QRect(aValue, (height()-mHandleHeight) / 2, mHandleWidth, mHandleHeight);
    else
        return QRect((width()-mHandleWidth) / 2, aValue, mHandleWidth, mHandleHeight);
}

void QvisRangeSlider::mousePressEvent(QMouseEvent* aEvent)
{
    if(aEvent->buttons() & Qt::LeftButton)
    {
        int pomheck = (mOrientation == Qt::Horizontal) ? aEvent->pos().y() : aEvent->pos().x();
        int posMax = (mOrientation == Qt::Horizontal) ? height() : width();
        int posValue = (mOrientation == Qt::Horizontal) ? aEvent->pos().x() : height() - aEvent->pos().y();
        int firstHandleRectPosValue  = (mOrientation == Qt::Horizontal) ? firstHandleRect().x()  : firstHandleRect().y()  + mHandleHeight / 2;
        int secondHandleRectPosValue = (mOrientation == Qt::Horizontal) ? secondHandleRect().x() : secondHandleRect().y() + mHandleHeight / 2;

        if(mType.testFlag(RightHandle) || mType.testFlag(DoubleHandles))
        {
            mFirstHandlePressed = firstHandleRect().contains(aEvent->pos());
            mSecondHandlePressed = false;
        }
        else
        {
            mFirstHandlePressed = false;

            if(mType.testFlag(RightHandle) || mType.testFlag(DoubleHandles))
                mSecondHandlePressed = secondHandleRect().contains(aEvent->pos());
            else
                mSecondHandlePressed = false;
        }

        if(mFirstHandlePressed)
        {
            emit lowerSliderPressed();
        }
        else if(mSecondHandlePressed)
        {
            emit upperSliderPressed();
        }

        int newLowerValue = (posValue - mMargin - mHandleSize / 2) * 1.0 / validLength() * mInterval + mMinimum;
        int newUpperValue = (posValue - mMargin - mHandleSize / 2 - (mType.testFlag(DoubleHandles) ? mHandleSize : 0)) * 1.0 / validLength() * mInterval + mMinimum;

        if(2 <= pomheck && pomheck <= posMax - 2)
        {
            if(posValue < firstHandleRectPosValue)
            {
                setLowerValue(newLowerValue);
                mFirstHandlePressed = true;
            }
            else if((posValue > firstHandleRectPosValue  || !mType.testFlag(LeftHandle)) &&
                    (posValue < secondHandleRectPosValue || !mType.testFlag(RightHandle)))
            {
                if(mType.testFlag(DoubleHandles))
                {
                    if(posValue - firstHandleRectPosValue < secondHandleRectPosValue - posValue)
                    {
                        setLowerValue(newLowerValue);
                        mFirstHandlePressed = true;
                    }
                    else
                    {
                        setUpperValue(newUpperValue);
                        mSecondHandlePressed = true;
                    }
                }
                else if(mType.testFlag(LeftHandle))
                {
                    setLowerValue(newLowerValue);
                    mFirstHandlePressed = true;
                }
                else if(mType.testFlag(RightHandle))
                {
                    setUpperValue(newUpperValue);
                    mSecondHandlePressed = true;
                }
            }
            else if(posValue > secondHandleRectPosValue)
            {
                setUpperValue(newUpperValue);
                mSecondHandlePressed = true;
            }
        }
    }
}

void QvisRangeSlider::mouseMoveEvent(QMouseEvent* aEvent)
{
    if(aEvent->buttons() & Qt::LeftButton)
    {
        int posValue = (mOrientation == Qt::Horizontal) ? aEvent->pos().x() : height() - aEvent->pos().y();
        int firstHandleRectPosValue  = (mOrientation == Qt::Horizontal) ? firstHandleRect().x()  : firstHandleRect().y();
        int secondHandleRectPosValue = (mOrientation == Qt::Horizontal) ? secondHandleRect().x() : secondHandleRect().y();

        if(mFirstHandlePressed && mType.testFlag(LeftHandle))
        {
            if(posValue + mHandleSize / 2 <= secondHandleRectPosValue)
            {
                setLowerValue((posValue - mMargin - mHandleSize / 2) * 1.0 / validLength() * mInterval + mMinimum);
            }
            else
            {
                setLowerValue(mUpperValue);
            }

            emit lowerSliderMoved(posValue);
        }
        else if(mSecondHandlePressed && mType.testFlag(RightHandle))
        {
            if(firstHandleRectPosValue + mHandleSize * (mType.testFlag(DoubleHandles) ? 1.5 : 0.5) <= posValue)
            {
                setUpperValue((posValue - mMargin - mHandleSize / 2 - (mType.testFlag(DoubleHandles) ? mHandleSize : 0)) * 1.0 / validLength() * mInterval + mMinimum);
            }
            else
            {
                setUpperValue(mLowerValue);
            }

            emit upperSliderMoved(posValue);
        }
    }
}

void QvisRangeSlider::mouseReleaseEvent(QMouseEvent* aEvent)
{
    Q_UNUSED(aEvent);

    if( mFirstHandlePressed )
        emit lowerSliderReleased();

    if( mSecondHandlePressed )
        emit upperSliderReleased();

    mFirstHandlePressed = false;
    mSecondHandlePressed = false;
}

void QvisRangeSlider::wheelEvent(QWheelEvent * aEvent)
{
    float delta = aEvent->angleDelta().y();
    float step = log(std::abs(delta));

    if( step < 2 )
        return;

    step = (step < 1) ? 1 : step;
    step = std::copysign(step, delta);

    if(aEvent->modifiers() & Qt::ShiftModifier)
    {
        step *= mSingleStep * 2; //mPageStep;
    }
    else
    {
        step *= mSingleStep;
    }

    int posCheck = (mOrientation == Qt::Horizontal) ? aEvent->position().y() : aEvent->position().x();
    int posMax = (mOrientation == Qt::Horizontal) ? height() : width();
    int posValue = (mOrientation == Qt::Horizontal) ? aEvent->position().x() : height() - aEvent->position().y();
    int firstHandleRectPosValue  = (mOrientation == Qt::Horizontal) ? firstHandleRect().x()  : firstHandleRect().y()  + mHandleHeight / 2;
    int secondHandleRectPosValue = (mOrientation == Qt::Horizontal) ? secondHandleRect().x() : secondHandleRect().y() + mHandleHeight / 2;

    if((mType.testFlag(LeftHandle) || mType.testFlag(DoubleHandles)) &&
            firstHandleRect().contains(aEvent->position()))
    {
        setLowerValue(mLowerValue + step);
    }
    else if((mType.testFlag(RightHandle) || mType.testFlag(DoubleHandles)) &&
            secondHandleRect().contains(aEvent->position()))
    {
        setUpperValue(mUpperValue + step);
    }
    else if(2 <= posCheck && posCheck <= posMax - 2)
    {
        if(posValue < firstHandleRectPosValue)
        {
            setLowerValue(mLowerValue + step);
        }
        else if((posValue > firstHandleRectPosValue  || !mType.testFlag(LeftHandle)) &&
                (posValue < secondHandleRectPosValue || !mType.testFlag(RightHandle)))
        {
            if(mType.testFlag(DoubleHandles))
            {
                if(posValue - firstHandleRectPosValue < secondHandleRectPosValue - posValue)
                {
                    setLowerValue(mLowerValue + step);
                }
                else
                {
                    setUpperValue(mUpperValue + step);
                }
            }
            else if(mType.testFlag(LeftHandle))
            {
                setLowerValue(mLowerValue + step);
            }
            else if(mType.testFlag(RightHandle))
            {
                setUpperValue(mUpperValue + step);
            }
        }
        else if(posValue > secondHandleRectPosValue)
        {
            setUpperValue(mUpperValue + step);
        }
    }
}

void QvisRangeSlider::changeEvent(QEvent* aEvent)
{
    if(aEvent->type() == QEvent::EnabledChange ||
       // <HACK> At creatation mouse tracking events is set to true.
       // This will cause an event which by the time it is handled the enable
       // status is known so the inital colors can be selected. </HACK>.
       aEvent->type() == QEvent::MouseTrackingChange)
    {
        if(isEnabled())
        {
            mSelectedGrooveBorderColor      = mSelectedGrooveBorderColorEnabled;
            mSelectedGrooveColor    = mSelectedGrooveColorEnabled;

            mUnselectedGrooveBorderColor    = mUnselectedGrooveBorderColorEnabled;
            mUnselectedGrooveColor  = mUnselectedGrooveColorEnabled;

            mHandleBorderColor        = mUnselectedGrooveBorderColorEnabled;
            mLeftHandleColor  = mLeftHandleColorEnabled;
            mRightHandleColor = mRightHandleColorEnabled;
        }
        else
        {
            mSelectedGrooveBorderColor      = mGrooveBorderColorDisabled;
            mSelectedGrooveColor    = mGrooveColorDisabled;

            mUnselectedGrooveBorderColor    = mGrooveBorderColorDisabled;
            mUnselectedGrooveColor  = mGrooveColorDisabled;

            mHandleBorderColor        = mGrooveBorderColorDisabled;
            mLeftHandleColor  = mHandleColorDisabled;
            mRightHandleColor = mHandleColorDisabled;
        }

        update();
    }
}

QSize QvisRangeSlider::minimumSizeHint() const
{
    if( mOrientation == Qt::Horizontal)
        return QSize(mHandleWidth * 2 + mMargin * 2, mHandleHeight);
    else
        return QSize(mHandleWidth, mHandleHeight * 2 + mMargin * 2);
}

int QvisRangeSlider::GetSingleStep() const
{
    return mSingleStep;
}
void QvisRangeSlider::SetSingleStep(int aStep)
{
    mSingleStep = aStep;
}

int QvisRangeSlider::GetPageStep() const
{
    return mPageStep;
}

void QvisRangeSlider::SetPageStep(int aStep)
{
    mPageStep = aStep;
}

int QvisRangeSlider::GetMinimum() const
{
    return mMinimum;
}

void QvisRangeSlider::SetMinimum(int aMinimum)
{
    setMinimum(aMinimum);
}

int QvisRangeSlider::GetMaximum() const
{
    return mMaximum;
}

void QvisRangeSlider::SetMaximum(int aMaximum)
{
    setMaximum(aMaximum);
}

int QvisRangeSlider::GetLowerValue() const
{
    return mLowerValue;
}

void QvisRangeSlider::SetLowerValue(int aLowerValue)
{
    setLowerValue(aLowerValue);
}

int QvisRangeSlider::GetUpperValue() const
{
    return mUpperValue;
}

void QvisRangeSlider::SetUpperValue(int aUpperValue)
{
    setUpperValue(aUpperValue);
}

void QvisRangeSlider::setLowerValue(int aLowerValue)
{
    if(aLowerValue > mUpperValue)
    {
        aLowerValue = mUpperValue;
    }

    if(aLowerValue < mMinimum)
    {
        aLowerValue = mMinimum;
    }

    if( mLowerValue != aLowerValue)
    {
        mLowerValue = aLowerValue;

        emit lowerValueChanged(mLowerValue);

        update();
    }
}

void QvisRangeSlider::setUpperValue(int aUpperValue)
{
    if(aUpperValue > mMaximum)
    {
        aUpperValue = mMaximum;
    }

    if(aUpperValue < mLowerValue)
    {
        aUpperValue = mLowerValue;
    }

    if(mUpperValue != aUpperValue)
    {
        mUpperValue = aUpperValue;

        emit upperValueChanged(mUpperValue);

        update();
    }
}

void QvisRangeSlider::setMinimum(int aMinimum)
{
    if(aMinimum == mMinimum)
    {
        return;
    }
    else if(aMinimum <= mMaximum)
    {
        mMinimum = aMinimum;
    }
    else
    {
        int oldMax = mMaximum;
        mMinimum = oldMax;
        mMaximum = aMinimum;
    }

    mInterval = mMaximum - mMinimum;
    update();

    setLowerValue(mMinimum);
    setUpperValue(mMaximum);
}

void QvisRangeSlider::setMaximum(int aMaximum)
{
    if(aMaximum == mMaximum)
    {
        return;
    }
    else if(aMaximum >= mMinimum)
    {
        mMaximum = aMaximum;
    }
    else
    {
        int oldMin = mMinimum;
        mMaximum = oldMin;
        mMinimum = aMaximum;
    }

    mInterval = mMaximum - mMinimum;
    update();

    setLowerValue(mMinimum);
    setUpperValue(mMaximum);
}

int QvisRangeSlider::validLength() const
{
    int len = (mOrientation == Qt::Horizontal) ? width() : height();
    return len - mMargin * 2 - mHandleSize * (mType.testFlag(DoubleHandles) ? 2 : 1);
}

void QvisRangeSlider::GetRange(int &aMinimum, int &aMaximum) const
{
    aMinimum = mMinimum;;
    aMaximum = mMaximum;
}

void QvisRangeSlider::SetRange(int aMinimum, int mMaximum)
{
    setMinimum(aMinimum);
    setMaximum(mMaximum);
}

void QvisRangeSlider::SetGrooveColor(QColor color)
{
    mSelectedGrooveColorEnabled = color;
    mSelectedGrooveBorderColorEnabled = color;

    if(isEnabled())
    {
        mSelectedGrooveColor = mSelectedGrooveColorEnabled;
        mSelectedGrooveBorderColor = mSelectedGrooveBorderColorEnabled;
    }
}

void QvisRangeSlider::SetLowerHandleColor(QColor color)
{
    mLeftHandleColorEnabled = color;

    if(isEnabled())
    {
        mLeftHandleColor = mLeftHandleColorEnabled;
    }
}

void QvisRangeSlider::SetUpperHandleColor(QColor color)
{
    mRightHandleColorEnabled = color;

    if(isEnabled())
    {
        mRightHandleColor = mRightHandleColorEnabled;
    }
}

Qt::Orientation QvisRangeSlider::GetOrientation() const
{
    return mOrientation;
}

void QvisRangeSlider::SetOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;
}

QvisRangeSlider::SliderHandles QvisRangeSlider::GetSliderHandle() const
{
    return mType;
}

void QvisRangeSlider::SetSliderHandle(QvisRangeSlider::SliderHandles type)
{
    mType = type;
}
