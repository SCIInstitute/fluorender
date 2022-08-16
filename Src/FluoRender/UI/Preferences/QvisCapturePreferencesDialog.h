#ifndef QVISCAPTUREPREFERENCESDIALOG_H
#define QVISCAPTUREPREFERENCESDIALOG_H

#include <QDialog>

class CapturePreferencesAttributes;

namespace Ui {
class QvisCapturePreferencesDialog;
}

class QvisCapturePreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QvisCapturePreferencesDialog(QWidget *parent = nullptr);
    ~QvisCapturePreferencesDialog();

    const CapturePreferencesAttributes * attributes() const;
    void setAttributes(CapturePreferencesAttributes *tmpAtts);

    const QString currentProjectDirectory() const;
    void setCurrentProjectDirectory(const QString &directory);

private slots:
    void FileNameTextChanged(const QString &text);
    void FileNameFamilyToggled(bool checked);
    void EmbedInProjectFolderToggled(bool checked);
    bool DirectoryTextReturnPressed();
    void DirectoryButtonClicked();

    void LZWCompressionToggled  (bool checked);
    void SaveAlphaChannelToggled(bool checked);
    void SaveFloatChannelToggled(bool checked);

    void ScaleOutputImageToggled(bool checked);
    void ScaleOutputImageSliderChanged(int value);
    void ScaleOutputImageSpinBoxChanged(double value);

    void CancelButtonClicked();
    void OkButtonClicked();

private:
    Ui::QvisCapturePreferencesDialog *ui;

    CapturePreferencesAttributes *atts{nullptr};
    CapturePreferencesAttributes *tmpAtts{nullptr};

    QString mCurrentProjectDirectory{tr("")};
};

#endif // QVISCAPTUREPREFERENCESDIALOG_H
