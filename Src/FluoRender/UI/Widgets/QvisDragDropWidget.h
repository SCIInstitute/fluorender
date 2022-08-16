#ifndef QvisDragDropWidget_H
#define QvisDragDropWidget_H

#include <QLabel>
#include <QWidget>
#include <QQueue>

class QDragEnterEvent;
class QDropEvent;
class QGridLayout;
class QLabel;

namespace Ui {
class QvisDragDropWidget;
}

class QvisDragDropWidget : public QWidget
{
    Q_OBJECT

public:
    // A drag and drop mode for controlling the drag and drop.
    enum DragDropMode {
        AllowDrag = 0x01,     // Allow widgets to be dragged
        AllowDropMove = 0x02, // Allow widgets to be moved with the window
        AllowDropCopy = 0x04, // Allow widgets to be dropped into the window
        AllowDragAndDropMove = AllowDrag | AllowDropMove,
        AllowDragAndDropCopy = AllowDrag | AllowDropCopy,
        AllowDrop = AllowDropMove | AllowDropCopy,
        AllowDragAndDrop = AllowDrag | AllowDrop,
    };
    Q_DECLARE_FLAGS(DragDropModes, DragDropMode)

    // Mode to determine what should be happen to original widget after an ignore drag event.
    enum DragDropIgnoreMode {
        ShowAfterIgnore,
        CloseAfterIgnore,
    };
    Q_DECLARE_FLAGS(DragDropIgnoreModes, DragDropIgnoreMode)

    // Mode to determine what should be happen to original widget after a copy drag event.
    enum DragDropCopyMode {
        ShowAfterCopy,
        CloseAfterCopy,
    };
    Q_DECLARE_FLAGS(DragDropCopyModes, DragDropCopyMode)

    explicit QvisDragDropWidget(QWidget *parent = nullptr);
    ~QvisDragDropWidget();

    // A kay so to prevent cross widget copies.
    int getKey() const;
    void setKey(int key);

    // A trash can icon so to remove a widget from with a window.
    bool trashCan() const;
    void addTrashCan();
    void removeTrashCan();

    // Sort widgets after adding or inserting (disables drop moves)
    bool getSorting() const;
    void setSorting(bool sorting);

    // Set the maximum number of columns in the grid layout.
    void setMaxColumns(int val);
    int  maxColumns();

    DragDropModes getDragDropMode() const;
    void setDragDropMode(DragDropModes mode);

    DragDropIgnoreModes getIgnoreMode() const;
    void setIgnoreMode(DragDropIgnoreModes mode);

    DragDropCopyModes getCopyMode() const;
    void setCopyMode(DragDropCopyModes mode);

    Qt::Orientation getOrientation();

    QLabel* getWidget(const QString name) const;
    QLabel* addWidget(const QString name, const QString pixmapName = QString(""));
    QLabel* addWidget(const QString name, const QPixmap &pixmap);
    QLabel* insertWidget(const QString before, const QString name, const QString pixmapName = QString(""));
    QLabel* insertWidget(const QString before, const QString name, const QPixmap &pixmap);
    void removeWidget(const QString name);
    QList<QString> getWdigetNames() const;

    void setWidgetBorder(const QString name, bool border, bool exclusive = true);

signals:
    void widgetClicked(const QString name);
    void widgetAdded  (const QString name);
    void widgetRemoved(const QString name);
    void widgetMoved  (const QString name);
    void widgetCopied (const QString name);

public slots:
    void setOrientation(Qt::Orientation orientation);

protected:
    void mousePressEvent  (QMouseEvent *event) override;
    void mouseMoveEvent   (QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent   (QDragEnterEvent *event) override;
    void dragMoveEvent    (QDragMoveEvent *event) override;
    void dropEvent        (QDropEvent *event) override;
    void resizeEvent      (QResizeEvent *event) override;

    void createDragEvent  (QMouseEvent *event);
    void updateLayout();

    QList<QLabel *> mDragWidgets;

    QLabel *mTrashCanLabel{nullptr};
    QPixmap mTrashCanPixmap;
    QPixmap mTrashCanTempPixmap;
    bool mMouseOverTrashCan{false};

    QLabel *mChildLabel{nullptr};
    QPixmap mChildPixmap;

    int mKey{0};
    bool mSorting{false};
    int  mMaxColumns{0};
    DragDropModes       mDragDropMode {AllowDragAndDrop};
    DragDropIgnoreModes mIgnoreMode   {ShowAfterIgnore};
    DragDropCopyModes   mCopyMode     {ShowAfterCopy};
    Qt::Orientation     mOrientation  {Qt::Horizontal};

private:
    Ui::QvisDragDropWidget *ui;

    QPoint mMouseDownPos;
};

#endif // QvisDragDropWidget_H
