#ifndef QVISOPENGLWIDGET_H
#define QVISOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class QvisOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit QvisOpenGLWidget(QWidget *parent = nullptr);
    ~QvisOpenGLWidget();

signals:
    void initializeWindow(const QString name);
    void paintWindow(const QString name);
    void resizeWindow(const QString name, const int w, const int h);

    void keyPress    (const QChar key);
    void keyRelease  (const QChar key);
    void mousePress  (const QPointF point);
    void mouseRelease(const QPointF point);
    void mouseMove   (const QPointF point);
    void activeWindow(const QString name);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void enterEvent(QEnterEvent * event) override;
    void leaveEvent(QEvent * event) override;

    void mousePressEvent(QMouseEvent* aEvent) override;
    void mouseMoveEvent(QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;

    void keyPressEvent(QKeyEvent* aEvent) override;
    void keyReleaseEvent(QKeyEvent* aEvent) override;

private:

    bool mMouseDown {false};
    bool mShiftDown {false};
    bool mCtrlDown  {false};
    bool mAltDown   {false};
    bool mMetaDown  {false};
};

#endif // QVISOPENGLWIDGET_H
