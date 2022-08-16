#ifndef QVISVIEWWINDOWDIALOG_H
#define QVISVIEWWINDOWDIALOG_H

#include "QvisDialogBase.h"

class QvisViewWindow;
class QvisOpenGLWidget;

namespace Ui {
class QvisViewWindowDialog;
}

class QvisViewWindowDialog : public QvisDialogBase
{
    Q_OBJECT

public:
    explicit QvisViewWindowDialog(QWidget *parent = nullptr);
    ~QvisViewWindowDialog();

    void updateWindow(bool doAll = true) override;

    void changeEvent(QEvent *aEvent) override;
    void closeEvent(QCloseEvent *aEvent) override;
    void keyPressEvent(QKeyEvent* aEvent) override;

    // Used when the ADS is NOT avabile as a dialog for the ViewWindow.
    void setWidget(QvisViewWindow *viewWindow);
    void removeWidget(QvisViewWindow *viewWindow);

    // Used as a full screen dialog for a SINGLE openGLWindow.
    void setWidget(QvisOpenGLWidget *openGLWindow);
    void removeWidget(QvisOpenGLWidget *openGLWindow);

signals:
//    void viewWindowDialogDestroyed(const QString name);
    void exitFullScreen();

//private slots:
//    void ViewWindowDialogDestroyed();
//    void Finished(int result);

private:
    Ui::QvisViewWindowDialog *ui;

    bool fullScreen {false};
};

#endif // QVISVIEWWINDOWDIALOG_H
