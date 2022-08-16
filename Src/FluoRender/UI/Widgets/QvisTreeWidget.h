#ifndef QVISTREEWIDGET_H
#define QVISTREEWIDGET_H

#include <QTreeWidget>

class QvisTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit QvisTreeWidget(QWidget *parent = nullptr);
    ~QvisTreeWidget();

    void setItemTypeIndex(int index);
    int  getItemTypeIndex() const;

    void setItemAcceptedTypeIndex(int index);
    int  getItemAcceptedTypeIndex() const;

signals:
    void iconClicked(QTreeWidgetItem *, int);
    void itemMoved(QTreeWidgetItem *, int, QTreeWidgetItem *, int);

protected:
    void mousePressEvent  (QMouseEvent* aEvent) override;
    void mouseMoveEvent   (QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;

    void dragEnterEvent   (QDragEnterEvent *event) override;
    void dragMoveEvent    (QDragMoveEvent  *event) override;
    void dropEvent        (QDropEvent      *event) override;

    void paintEvent       (QPaintEvent *event) override;

    bool canDrop(QPoint position);

private:
    bool mCanDrop{false};
    int  mItemTypeIndex{-1};
    int  mItemAcceptedIndex{-1};

};

#endif // QVISTREEWIDGET_H
