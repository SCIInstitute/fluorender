#ifndef QVISRANGESLIDER_H
#define QVISRANGESLIDER_H

#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

class QDESIGNER_WIDGET_EXPORT QvisRangeSlider : public QWidget
{
    Q_OBJECT

public:
    enum SliderHandle { NoHandle = 0x0,
                        LeftHandle = 0x1,
                        RightHandle = 0x2,
                        DoubleHandles = LeftHandle | RightHandle };

    Q_ENUM(SliderHandle)
    Q_DECLARE_FLAGS(SliderHandles, SliderHandle)

    Q_PROPERTY(int minimum    READ GetMinimum    WRITE SetMinimum    CONSTANT)
    Q_PROPERTY(int maximum    READ GetMaximum    WRITE SetMaximum    CONSTANT)
    Q_PROPERTY(int singleStep READ GetSingleStep WRITE SetSingleStep CONSTANT)
    Q_PROPERTY(int pageStep   READ GetPageStep   WRITE SetPageStep   CONSTANT)
    Q_PROPERTY(int lowerValue READ GetLowerValue WRITE SetLowerValue NOTIFY lowerValueChanged)
    Q_PROPERTY(int upperValue READ GetUpperValue WRITE SetUpperValue NOTIFY upperValueChanged)
    Q_PROPERTY(Qt::Orientation orientation READ GetOrientation  WRITE SetOrientation  CONSTANT)
    Q_PROPERTY(QvisRangeSlider::SliderHandles handle READ GetSliderHandle WRITE SetSliderHandle CONSTANT)

    explicit QvisRangeSlider(QWidget *parent = Q_NULLPTR);
    explicit QvisRangeSlider(Qt::Orientation orientation, SliderHandles type = DoubleHandles, QWidget* aParent = Q_NULLPTR);

    QSize minimumSizeHint() const override;

    int  GetMinimum() const;
    void SetMinimum(int aMinimum);

    int  GetMaximum() const;
    void SetMaximum(int aMaximum);

    int  GetLowerValue() const;
    void SetLowerValue(int aLowerValue);

    int  GetUpperValue() const;
    void SetUpperValue(int aUpperValue);

    int  GetSingleStep() const;
    void SetSingleStep(int aStep);

    int  GetPageStep() const;
    void SetPageStep(int aStep);

    Qt::Orientation GetOrientation() const;
    void SetOrientation(Qt::Orientation orientation);

    SliderHandles GetSliderHandle() const;
    void SetSliderHandle(SliderHandles type);

    void GetRange(int &aMinimum, int &aMaximum) const;
    void SetRange(int aMinimum, int aMaximum);

    void SetGrooveColor     (QColor color);
    void SetLowerHandleColor(QColor color);
    void SetUpperHandleColor(QColor color);

protected:
    void paintEvent(QPaintEvent* aEvent) override;
    void mousePressEvent(QMouseEvent* aEvent) override;
    void mouseMoveEvent(QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;
    void changeEvent(QEvent* aEvent) override;
    void wheelEvent(QWheelEvent * event) override;

    QRectF firstHandleRect() const;
    QRectF secondHandleRect() const;
    QRectF handleRect(int aValue) const;

signals:
    void lowerSliderMoved(int aLowerPos);
    void upperSliderMoved(int aUpperPos);

    void lowerSliderPressed();
    void upperSliderPressed();

    void lowerSliderReleased();
    void upperSliderReleased();

    void lowerValueChanged(int aLowerValue);
    void upperValueChanged(int aUpperValue);

public slots:
    void setLowerValue(int aLowerValue);
    void setUpperValue(int aUpperValue);
    void setMinimum(int aMinimum);
    void setMaximum(int aMaximum);

private:
    Q_DISABLE_COPY(QvisRangeSlider)

    float currentPercentage();
    int validLength() const;

    // Properties
    Qt::Orientation mOrientation{Qt::Horizontal};
    SliderHandles mType{DoubleHandles};

    int mMinimum{0};
    int mMaximum{99};
    int mLowerValue{0};
    int mUpperValue{99};
    int mSingleStep{1};
    int mPageStep{10};

    // Internal helpers
    bool mFirstHandlePressed{false};
    bool mSecondHandlePressed{false};
    int mInterval{mMaximum - mMinimum};

    // Style variables
    // Border color for the groove
    QColor mGrooveBorderColorDisabled{QColor(0xD5, 0xD5, 0xD5)};
    QColor mSelectedGrooveBorderColorEnabled{QColor(0x00, 0x7A, 0xFC)};
    QColor mUnselectedGrooveBorderColorEnabled{QColor(0xCD, 0xCD, 0xCD)};
    QColor mSelectedGrooveBorderColor{mSelectedGrooveBorderColorEnabled};
    QColor mUnselectedGrooveBorderColor{mUnselectedGrooveBorderColorEnabled};

    // Background color for the groove
    QColor mGrooveColorDisabled{QColor(0xD8, 0xD8, 0xD8)};
    QColor mSelectedGrooveColorEnabled{QColor(0x00, 0x7E, 0xFF)};
    QColor mUnselectedGrooveColorEnabled{QColor(0xD3, 0xD3, 0xD3)};
    QColor mSelectedGrooveColor{mSelectedGrooveColorEnabled};
    QColor mUnselectedGrooveColor{mUnselectedGrooveColorEnabled};

    // Border and Background color for the handles.
    QColor mHandleBorderColor{mUnselectedGrooveBorderColorEnabled};
    QColor mHandleColorDisabled{QColor(0xF7, 0xF7, 0xF7)};
    QColor mLeftHandleColorEnabled{QColor(0xFF, 0xFF, 0xFF)};
    QColor mRightHandleColorEnabled{QColor(0xFF, 0xFF, 0xFF)};
    QColor mLeftHandleColor{mLeftHandleColorEnabled};
    QColor mRightHandleColor{mRightHandleColorEnabled};

    int mGrooveHeight = 3;
    int mHandleWidth  = 20;
    int mHandleHeight = 20;
    int mHandleBorderRadius = 10;
    int mMargin = 1;
    int mHandleSize = mHandleWidth;  // Always mHandleWidth
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QvisRangeSlider::SliderHandles)

#endif // RANGESLIDER_H
