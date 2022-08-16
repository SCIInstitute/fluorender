#ifndef QVISMESSAGEDIALOG_H
#define QVISMESSAGEDIALOG_H

#include <QDialog>

namespace Ui {
class QvisMessageDialog;
}

class QvisMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QvisMessageDialog(QWidget *parent = nullptr);
    ~QvisMessageDialog();

    QAction * toggleViewAction();
    void setToggleViewAction(QAction * toggleViewAction);

public slots:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();
    void toggleView(bool show = true);

protected:
    QAction * mToggleViewAction{nullptr};

private:
    Ui::QvisMessageDialog *ui;

};

#endif // QVISMESSAGEDIALOG_H
