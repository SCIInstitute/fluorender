#ifndef QVISMOVIEPREFERENCESDIALOG_H
#define QVISMOVIEPREFERENCESDIALOG_H

#include <QDialog>

class MoviePreferencesAttributes;

namespace Ui {
class QvisMoviePreferencesDialog;
}

class QvisMoviePreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QvisMoviePreferencesDialog(QWidget *parent = nullptr);
    ~QvisMoviePreferencesDialog();

    const MoviePreferencesAttributes * attributes() const;
    void setAttributes(MoviePreferencesAttributes *tmpAtts);

    const QString currentProjectDirectory() const;
    void setCurrentProjectDirectory(const QString &directory);

private slots:
    void FileNameTextChanged(const QString &text);
    void EmbedInProjectFolderToggled(bool checked);
    bool DirectoryTextReturnPressed();
    void DirectoryButtonClicked();

    void FileTypeIndexChanged(int index);
    void LZWCompressionToggled  (bool checked);
    void SaveAlphaChannelToggled(bool checked);
    void SaveFloatChannelToggled(bool checked);
    void BitRateSpinBoxValueChanged(double value);

    void ScaleOutputImageToggled(bool checked);
    void ScaleOutputImageSliderChanged(int value);
    void ScaleOutputImageSpinBoxChanged(double value);

    void CancelButtonClicked();
    void OkButtonClicked();

private:
    Ui::QvisMoviePreferencesDialog *ui;

    MoviePreferencesAttributes *atts{nullptr};
    MoviePreferencesAttributes *tmpAtts{nullptr};

    QString mCurrentProjectDirectory{tr("")};
};

#endif // QVISMOVIEPREFERENCESDIALOG_H
