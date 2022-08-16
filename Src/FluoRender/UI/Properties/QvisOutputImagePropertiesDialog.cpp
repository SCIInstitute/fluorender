#include "QvisOutputImagePropertiesDialog.h"
#include "ui_QvisOutputImagePropertiesDialog.h"

QvisOutputImagePropertiesDialog::QvisOutputImagePropertiesDialog(QWidget *parent) :
    QvisDialogBase(parent),
    ui(new Ui::QvisOutputImagePropertiesDialog)
{
    ui->setupUi(this);
}

QvisOutputImagePropertiesDialog::~QvisOutputImagePropertiesDialog()
{
    delete ui;
}

void QvisOutputImagePropertiesDialog::updateWindow(bool doAll)
{
    for(int i = 0; i < mAttributes->numAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!mAttributes->isSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
        case OutputImagePropertiesAttributes::ID_RedGamma:
        {
            double value = ui->RedGammaSpinBox->value();

            const QSignalBlocker blocker0(ui->RedGammaSlider);
            ui->RedGammaSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->RedGammaSpinBox);
            ui->RedGammaSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_GreenGamma:
        {
            double value = ui->GreenGammaSpinBox->value();

            const QSignalBlocker blocker0(ui->GreenGammaSlider);
            ui->GreenGammaSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->GreenGammaSpinBox);
            ui->GreenGammaSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_BlueGamma:
        {
            double value = ui->BlueGammaSpinBox->value();

            const QSignalBlocker blocker0(ui->BlueGammaSlider);
            ui->BlueGammaSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->BlueGammaSpinBox);
            ui->BlueGammaSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_RedLuminance:
        {
            double value = ui->RedLuminanceSpinBox->value();

            const QSignalBlocker blocker0(ui->RedLuminanceSlider);
            ui->RedLuminanceSlider->setValue(value);

            const QSignalBlocker blocker1(ui->RedLuminanceSpinBox);
            ui->RedLuminanceSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_GreenLuminance:
        {
            double value = ui->GreenLuminanceSpinBox->value();

            const QSignalBlocker blocker0(ui->GreenLuminanceSlider);
            ui->GreenLuminanceSlider->setValue(value);

            const QSignalBlocker blocker1(ui->GreenLuminanceSpinBox);
            ui->GreenLuminanceSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_BlueLuminance:
        {
            double value = ui->BlueLuminanceSpinBox->value();
            const QSignalBlocker blocker0(ui->BlueLuminanceSlider);
            ui->BlueLuminanceSlider->setValue(value);

            const QSignalBlocker blocker1(ui->BlueLuminanceSpinBox);
            ui->BlueLuminanceSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_RedEqualization:
        {
            double value = ui->RedEqualizationSpinBox->value();

            const QSignalBlocker blocker0(ui->RedEqualizationSlider);
            ui->RedEqualizationSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->RedEqualizationSpinBox);
            ui->RedEqualizationSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_GreenEqualization:
        {
            double value = ui->GreenEqualizationSpinBox->value();

            const QSignalBlocker blocker0(ui->GreenEqualizationSlider);
            ui->GreenEqualizationSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->GreenEqualizationSpinBox);
            ui->GreenEqualizationSpinBox->setValue(value);
        }
            break;

        case OutputImagePropertiesAttributes::ID_BlueEqualization:
        {
            double value = ui->BlueEqualizationSlider->value();

            const QSignalBlocker blocker0(ui->BlueEqualizationSlider);
            ui->BlueEqualizationSlider->setValue(value*100.0);

            const QSignalBlocker blocker1(ui->BlueEqualizationSpinBox);
            ui->BlueEqualizationSpinBox->setValue(value);
        }
            break;

        }
    }
}

void QvisOutputImagePropertiesDialog::ColorTabChanged(int index)
{
    mCurrentTab = COLOR(index);
}

void QvisOutputImagePropertiesDialog::ResetAllClicked()
{
    this->notify = false;
    {
        const QSignalBlocker blocker0(ui->RedGammaSpinBox);
        const QSignalBlocker blocker1(ui->RedLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->RedEqualizationSpinBox);

        ui->RedGammaSpinBox->setValue(mRedDefaultGamma);
        ui->RedLuminanceSpinBox->setValue(mRedDefaultLuminance);
        ui->RedEqualizationSpinBox->setValue(mRedDefaultEqualization);
    }
    {
        const QSignalBlocker blocker0(ui->GreenGammaSpinBox);
        const QSignalBlocker blocker1(ui->GreenLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->GreenEqualizationSpinBox);

        ui->GreenGammaSpinBox->setValue(mGreenDefaultGamma);
        ui->GreenLuminanceSpinBox->setValue(mGreenDefaultLuminance);
        ui->GreenEqualizationSpinBox->setValue(mGreenDefaultEqualization);
    }
    {
        const QSignalBlocker blocker0(ui->BlueGammaSpinBox);
        const QSignalBlocker blocker1(ui->BlueLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->BlueEqualizationSpinBox);

        ui->BlueGammaSpinBox->setValue(mBlueDefaultGamma);
        ui->BlueLuminanceSpinBox->setValue(mBlueDefaultLuminance);
        ui->BlueEqualizationSpinBox->setValue(mBlueDefaultEqualization);
    }

    this->notify = true;

    sendNotification();
}

void QvisOutputImagePropertiesDialog::MakeDefaultClicked()
{
    mRedDefaultGamma        = ui->RedGammaSpinBox->value();
    mRedDefaultLuminance    = ui->RedLuminanceSpinBox->value();
    mRedDefaultEqualization = ui->RedEqualizationSpinBox->value();

    mGreenDefaultGamma        = ui->GreenGammaSpinBox->value();
    mGreenDefaultLuminance    = ui->GreenLuminanceSpinBox->value();
    mGreenDefaultEqualization = ui->GreenEqualizationSpinBox->value();

    mBlueDefaultGamma        = ui->BlueGammaSpinBox->value();
    mBlueDefaultLuminance    = ui->BlueLuminanceSpinBox->value();
    mBlueDefaultEqualization = ui->BlueEqualizationSpinBox->value();
}

// Used for Red, Green, and Blue
void QvisOutputImagePropertiesDialog::GammaSliderChanged(int value)
{
    this->GammaValueChanged(double(value)/100.0);
}

void QvisOutputImagePropertiesDialog::GammaValueChanged(double value)
{
    if( setRed() )
    {
        const QSignalBlocker blocker0(ui->RedGammaSlider);
        ui->RedGammaSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->RedGammaSpinBox);
        ui->RedGammaSpinBox->setValue(value);
        ui->RedGammaSpinBox->setFocus();
    }

    if( setGreen() )
    {
        const QSignalBlocker blocker0(ui->GreenGammaSlider);
        ui->GreenGammaSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->GreenGammaSpinBox);
        ui->GreenGammaSpinBox->setValue(value);
        ui->GreenGammaSpinBox->setFocus();
    }

    if( setBlue() )
    {
        const QSignalBlocker blocker0(ui->BlueGammaSlider);
        ui->BlueGammaSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->BlueGammaSpinBox);
        ui->BlueGammaSpinBox->setValue(value);
        ui->BlueGammaSpinBox->setFocus();
    }

    this->sendNotification();
}

void QvisOutputImagePropertiesDialog::LuminanceSliderChanged(int value)
{
    this->LuminanceValueChanged(value);
}

void QvisOutputImagePropertiesDialog::LuminanceValueChanged(int value)
{
    if( setRed() )
    {
        const QSignalBlocker blocker0(ui->RedLuminanceSlider);
        ui->RedLuminanceSlider->setValue(value);

        const QSignalBlocker blocker1(ui->RedLuminanceSpinBox);
        ui->RedLuminanceSpinBox->setValue(value);
        ui->RedLuminanceSpinBox->setFocus();
    }

    if( setGreen() )
    {
        const QSignalBlocker blocker0(ui->GreenLuminanceSlider);
        ui->GreenLuminanceSlider->setValue(value);

        const QSignalBlocker blocker1(ui->GreenLuminanceSpinBox);
        ui->GreenLuminanceSpinBox->setValue(value);
        ui->GreenLuminanceSpinBox->setFocus();
    }

    if( setBlue() )
    {
        const QSignalBlocker blocker0(ui->BlueLuminanceSlider);
        ui->BlueLuminanceSlider->setValue(value);

        const QSignalBlocker blocker1(ui->BlueLuminanceSpinBox);
        ui->BlueLuminanceSpinBox->setValue(value);
        ui->BlueLuminanceSpinBox->setFocus();
    }

    this->sendNotification();
}

void QvisOutputImagePropertiesDialog::EqualizationSliderChanged(int value)
{
    this->EqualizationValueChanged(double(value)/100.0);
}

void QvisOutputImagePropertiesDialog::EqualizationValueChanged(double value)
{
    if( setRed() )
    {
        const QSignalBlocker blocker0(ui->RedEqualizationSlider);
        ui->RedEqualizationSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->RedEqualizationSpinBox);
        ui->RedEqualizationSpinBox->setValue(value);
        ui->RedEqualizationSpinBox->setFocus();
    }

    if( setGreen() )
    {
        const QSignalBlocker blocker0(ui->GreenEqualizationSlider);
        ui->GreenEqualizationSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->GreenEqualizationSpinBox);
        ui->GreenEqualizationSpinBox->setValue(value);
        ui->GreenEqualizationSpinBox->setFocus();
    }

    if( setBlue() )
    {
        const QSignalBlocker blocker0(ui->BlueEqualizationSlider);
        ui->BlueEqualizationSlider->setValue(value*100.0);

        const QSignalBlocker blocker1(ui->BlueEqualizationSpinBox);
        ui->BlueEqualizationSpinBox->setValue(value);
        ui->BlueEqualizationSpinBox->setFocus();
    }

    this->sendNotification();
}

void QvisOutputImagePropertiesDialog::LinkColorToggled(bool checked)
{
    double gamma;
    int luminence;
    double equalization;

    bool update = false;

    if(mCurrentTab == QvisOutputImagePropertiesDialog::RED)
    {
        mLinkRed = checked;

        // Propigate to other colors if they are also linked.
        if(mLinkRed && (mLinkGreen || mLinkBlue))
        {
            gamma        = ui->RedGammaSpinBox->value();
            luminence    = ui->RedLuminanceSpinBox->value();
            equalization = ui->RedEqualizationSpinBox->value();

            update = true;
        }
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::GREEN)
    {
        mLinkGreen = checked;

        // Propigate to other colors if they are also linked.
        if(mLinkGreen && (mLinkRed || mLinkBlue))
        {
            gamma        = ui->GreenGammaSpinBox->value();
            luminence    = ui->GreenLuminanceSpinBox->value();
            equalization = ui->GreenEqualizationSpinBox->value();

            update = true;
        }
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::BLUE)
    {
        mLinkBlue = checked;

        // Propigate to other colors if they are also linked.
        if(mLinkBlue && (mLinkRed || mLinkGreen))
        {
            gamma        = ui->BlueGammaSpinBox->value();
            luminence    = ui->BlueLuminanceSpinBox->value();
            equalization = ui->BlueEqualizationSpinBox->value();

            update = true;
        }
    }

    // Propigate to other colors if they are also linked.
    if( update )
    {
        this->notify = false;
        this->GammaValueChanged(gamma);
        this->LuminanceValueChanged(luminence);
        this->EqualizationValueChanged(equalization);
        this->notify = true;

        this->sendNotification();
    }
}

void QvisOutputImagePropertiesDialog::ResetColorClicked()
{
    this->notify = false;

    if(mCurrentTab == QvisOutputImagePropertiesDialog::RED)
    {
        const QSignalBlocker blocker0(ui->RedGammaSpinBox);
        const QSignalBlocker blocker1(ui->RedLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->RedEqualizationSpinBox);

        ui->RedGammaSpinBox->setValue(mRedDefaultGamma);
        ui->RedLuminanceSpinBox->setValue(mRedDefaultLuminance);
        ui->RedEqualizationSpinBox->setValue(mRedDefaultEqualization);
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::GREEN)
    {
        const QSignalBlocker blocker0(ui->GreenGammaSpinBox);
        const QSignalBlocker blocker1(ui->GreenLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->GreenEqualizationSpinBox);

        ui->GreenGammaSpinBox->setValue(mGreenDefaultGamma);
        ui->GreenLuminanceSpinBox->setValue(mGreenDefaultLuminance);
        ui->GreenEqualizationSpinBox->setValue(mGreenDefaultEqualization);
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::BLUE)
    {
        const QSignalBlocker blocker0(ui->BlueGammaSpinBox);
        const QSignalBlocker blocker1(ui->BlueLuminanceSpinBox);
        const QSignalBlocker blocker2(ui->BlueEqualizationSpinBox);

        ui->BlueGammaSpinBox->setValue(mBlueDefaultGamma);
        ui->BlueLuminanceSpinBox->setValue(mBlueDefaultLuminance);
        ui->BlueEqualizationSpinBox->setValue(mBlueDefaultEqualization);
    }

    this->notify = true;

    this->sendNotification();
}

bool QvisOutputImagePropertiesDialog::setRed()
{
    bool setRed = false;

    if(mCurrentTab == QvisOutputImagePropertiesDialog::RED)
    {
        setRed = true;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::GREEN)
    {
        setRed = mLinkRed && mLinkGreen;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::BLUE)
    {
        setRed = mLinkRed && mLinkBlue;
    }

    return setRed;
}

bool QvisOutputImagePropertiesDialog::setGreen()
{
    bool setGreen = false;

    if(mCurrentTab == QvisOutputImagePropertiesDialog::RED)
    {
        setGreen = mLinkGreen && mLinkRed;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::GREEN)
    {
        setGreen = true;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::BLUE)
    {
        setGreen = mLinkGreen && mLinkBlue;
    }

    return setGreen;
}

bool QvisOutputImagePropertiesDialog::setBlue()
{
    bool setBlue = false;

    if(mCurrentTab == QvisOutputImagePropertiesDialog::RED)
    {
        setBlue = mLinkBlue && mLinkRed;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::GREEN)
    {
        setBlue = mLinkBlue && mLinkGreen;
    }
    else if(mCurrentTab == QvisOutputImagePropertiesDialog::BLUE)
    {
        setBlue = true;
    }

    return setBlue;
}
