#ifndef RENDER_CANVAS_HPP
#define RENDER_CANVAS_HPP

#include <QWindow>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QScreen>

class RenderCanvas : public QWindow, protected QOpenGLFunctions
{
  Q_OBJECT

  public:
    explicit RenderCanvas(QWindow *parent = nullptr);
    //~RenderCanvas();

    virtual void render(QPainter *painter);
    virtual void render();

    virtual void initialize();

    void setAnimating(bool animating);
    void setCurrentContext();

  public slots:
    void renderLater();
    void renderNow();

  protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

  private:
    bool m_animating;

    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;

};


#endif
