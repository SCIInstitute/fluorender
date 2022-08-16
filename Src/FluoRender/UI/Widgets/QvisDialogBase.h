#ifndef QVISDIALOGBASE_H
#define QVISDIALOGBASE_H

#include <QDialog>

class QAction;
class QToolButton;

class QvisDragDropToolBar;

class QvisDialogBase : public QDialog
{
    Q_OBJECT

public:
    explicit QvisDialogBase(QWidget *parent = nullptr);
    ~QvisDialogBase();

    virtual void closeEvent(QCloseEvent *e) override;

    virtual void updateWindow(bool doAll = true) = 0;

    bool hasDragDropToolBar() {return mDragDropToolbar != nullptr;};
    QvisDragDropToolBar * dragDropToolBar();
    QAction* addDragAction(QToolButton *button, const char* member, bool checkable = false);

    QAction * toggleViewAction();
    void setToggleViewAction(QAction * toggleViewAction);

    Qt::Orientation toolbarActionOrientation() const;
    void setToolbarActionOrientation(Qt::Orientation val);

signals:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();

    void currentCursorMode(QString tool, QString mode);

public slots:
    void toggleView(bool show = true);
    void showDragDropToolBar();

protected:
    QAction * mToggleViewAction{nullptr};
    QvisDragDropToolBar * mDragDropToolbar{nullptr};
    QAction * mToolbarAction{nullptr};

    // Methods
    void sendNotification();

    Qt::Orientation mToolbarActionOrientation{Qt::Vertical};
    bool notify{true};
    int  notifyCount{0};
};

#endif // QVISDIALOGBASE_H
