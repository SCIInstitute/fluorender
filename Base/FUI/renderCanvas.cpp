#include "renderCanvas.hpp"

RenderCanvas::RenderCanvas(QWindow *parent) : QWindow(parent), m_animating(false),m_context(nullptr),m_device(nullptr)
{
  setSurfaceType(QWindow::OpenGLSurface);
}

void RenderCanvas::render(QPainter *painter)
{
  Q_UNUSED(painter);
}

void RenderCanvas::initialize()
{
}

void RenderCanvas::render()
{
  if(!m_device)
    m_device = new QOpenGLPaintDevice;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_device->setSize(size() * devicePixelRatio());
  m_device->setDevicePixelRatio(devicePixelRatio());

  QPainter painter(m_device);
  render(&painter);
}

void RenderCanvas::renderLater()
{
  requestUpdate();
}

bool RenderCanvas::event(QEvent *event)
{
  switch(event->type())
  {
    case QEvent::UpdateRequest:
      renderNow();
      return true;
    default:
      return QWindow::event(event);
  }
}

void RenderCanvas::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event);

  if(isExposed())
    renderNow();
}

void RenderCanvas::setAnimating(bool animating)
{
  m_animating = animating;

  if(animating)
    renderLater();
}

void RenderCanvas::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}

void RenderCanvas::setCurrentContext()
{
  m_context->makeCurrent(this);
}
