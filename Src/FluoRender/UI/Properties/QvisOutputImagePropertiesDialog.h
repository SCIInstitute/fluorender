#ifndef QVISOUTPUTIMAGEPROPERTIESDIALOG_H
#define QVISOUTPUTIMAGEPROPERTIESDIALOG_H

#include "QvisDialogBase.h"

namespace Ui {
class QvisOutputImagePropertiesDialog;
}

class OutputImagePropertiesAttributes
{
public:

    enum {
        ID_RedGamma = 0,
        ID_RedLuminance,
        ID_RedEqualization,

        ID_GreenGamma,
        ID_GreenLuminance,
        ID_GreenEqualization,

        ID_BlueGamma,
        ID_BlueLuminance,
        ID_BlueEqualization,
        ID_LAST_VARIABLE
    };

    bool isSelected(int i) { return false; };
    int numAttributes() { return ID_LAST_VARIABLE; };
};

class QvisOutputImagePropertiesDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisOutputImagePropertiesDialog(QWidget *parent = nullptr);
    ~QvisOutputImagePropertiesDialog();

    void updateWindow(bool doAll = true) override;

private slots:
    void ColorTabChanged(int index);

    void ResetAllClicked();
    void MakeDefaultClicked();

    // Used for Red, Green, and Blue
    void GammaSliderChanged(int value);
    void GammaValueChanged(double value);
    void LuminanceSliderChanged(int value);
    void LuminanceValueChanged(int value);
    void EqualizationSliderChanged(int value);
    void EqualizationValueChanged(double value);

    void LinkColorToggled(bool checked);
    void ResetColorClicked();

private:
    Ui::QvisOutputImagePropertiesDialog *ui{nullptr};

    enum COLOR { RED = 0, GREEN = 1, BLUE = 2 };

    // Methods
    bool setRed  ();
    bool setGreen();
    bool setBlue ();

    bool notify{true};
    int  notifyCount{0};

    // Variables
    double mRedDefaultGamma{1.00};
    int    mRedDefaultLuminance{35};
    double mRedDefaultEqualization{0.00};

    double mGreenDefaultGamma{1.00};
    int    mGreenDefaultLuminance{35};
    double mGreenDefaultEqualization{0.00};

    double mBlueDefaultGamma{1.00};
    int    mBlueDefaultLuminance{35};
    double mBlueDefaultEqualization{0.00};

    COLOR  mCurrentTab{RED};

    bool mLinkRed{  false};
    bool mLinkGreen{false};
    bool mLinkBlue {false};

    // Variables
    OutputImagePropertiesAttributes* mAttributes{nullptr};
};

#endif // QVISOUTPUTIMAGEPROPERTIESDIALOG_H
