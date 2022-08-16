#ifndef QVISDRAGDROPTOOLBAR_H
#define QVISDRAGDROPTOOLBAR_H

#include <QToolBar>
#include <QString>

class QDialog;
class QPushButton;
class QToolButton;
class QVBoxLayout;

class QvisDragDropWidget;

class QvisDragDropToolBar : public QToolBar
{
    Q_OBJECT

public:
     QvisDragDropToolBar(QString name, QWidget *parent = nullptr, QWidget *dragParent = nullptr);
    ~QvisDragDropToolBar();

     void     addDragAction(QAction* action);
     QAction* addDragAction(const QString &text);
     QAction* addDragAction(const QIcon &icon, const QString &text);
     QAction* addDragAction(const QString &text,
                                  const QObject *receiver, const char* member);
     QAction* addDragAction(const QIcon &icon, const QString &text,
                                  const QObject *receiver, const char* member);
     void insertDragAction(QAction *before, QAction *action);
     void removeDragAction(QAction *action);

     QByteArray saveState(int version = 0) const;
     bool restoreState(const QByteArray &state, int version = 0);

     void setIcon(QIcon icon);

     void setPixmapSize(int val);
     int  pixmapSize() const;

     void setMaxColumns(int val);
     int  maxColumns() const;

     void setSortSourceAction(bool val);
     bool sortSourceAction() const;

     void setImmediatelyAddSourceAction(bool val);
     bool immediatelyAddSourceAction() const;

     void setKeepSourceAction(bool val);
     bool keepSourceAction() const;

     void setToggleCheckableActions(bool val);
     bool toggleCheckableActions() const;

     void setBorderCheckableActions(bool val);
     bool borderCheckableActions() const;

     void setExclusiveCheckableActions(bool val);
     bool exclusiveCheckableActions() const;

     void setGarbageCan(bool val);
     bool garbageCan() const;

     QToolBar *dragToolBar() const;

signals:
     void toggleParentView();

public slots:
    void toggleDragDialog(bool show = false);

private slots:
    void widgetClicked(const QString name);
    void widgetRemoved(const QString name);

private:
    QAction*            mDropAction{nullptr};
    QvisDragDropWidget* mDropWidget{nullptr};
    QDialog*            mDragDialog       {nullptr};
    QVBoxLayout*        mDragLayout       {nullptr};
    QToolBar*           mDragToolbar      {nullptr};
    QvisDragDropWidget* mDragWidget       {nullptr};

    int  mPixmapSize{24};
    int  mMaxColumns{0};
    bool mSortSourceAction          {false};
    bool mKeepSourceAction          {true};
    bool mToggleCheckableActions    {true};
    bool mBorderCheckableActions    {false};
    bool mExclusiveCheckableActions {false};
    bool mImmediatelyAddSourceAction{true};
    bool mGarbageCan                {false};

    bool mRestoringState{false};
    int  mCurrentVersion{1};

    std::map<QString, QAction *> mActions;
};

#endif // QVISDRAGDROPTOOLBAR_H
