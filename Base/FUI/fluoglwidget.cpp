#include "fluoglwidget.hpp"

//FluoGLWidget::FluoGLWidget(QWidget *parent, bool ifMain) : QOpenGLWidget (parent)
FluoGLWidget::FluoGLWidget(QWidget *parent, bool ifMain)
{
  // initializeGL();
  isMain = ifMain;
}

void FluoGLWidget::initializeGL()
{

  float r,g,b;
  float a = normalize_0_1(RGB_MAX,RGB_MIN,RGB_MAX);

  initializeOpenGLFunctions();
  qColortoRGB(Qt::black,r,g,b);
  glClearColor(r,g,b,a);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

}

void FluoGLWidget::paintGL()
{
  float r,g,b;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(isMain)
  {
    glBegin(GL_TRIANGLES);
    qColortoRGB(Qt::cyan,r,g,b);
    renderTriangle(r,g,b);
    glEnd();
  }
  else
  {
    glBegin(GL_POLYGON);
    qColortoRGB(Qt::red,r,g,b);
    renderHexagon(r,g,b);
    glEnd();
  }
}

void FluoGLWidget::resizeGL(int w, int h)
{
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void FluoGLWidget::qColortoRGB(const QColor &C, float &r, float &g, float &b) const
{
  r = normalize_0_1(C.red(),RGB_MIN,RGB_MAX);
  g = normalize_0_1(C.green(),RGB_MIN,RGB_MAX);
  b = normalize_0_1(C.blue(),RGB_MIN,RGB_MAX);
}

float FluoGLWidget::normalize_0_1(float val, float min, float max) const
{
  return (val - min)/(max - min);
}

void FluoGLWidget::renderTriangle(float r, float g, float b)
{
  glColor3f(r,g,b);
  glVertex3f(0.0f,1.0f,0.0f);
  glVertex3f(-0.5f,0.0f,0.0f);
  glVertex3f(0.5f,0.0f,0.0f);
}

void FluoGLWidget::renderHexagon(float r, float g, float b)
{
  glColor3f(r,g,b);
  glVertex3f(0.0f,0.5f,-1.0f);
  glVertex3f(-0.8f,0.0f,-1.0f);
  glVertex3f(-0.4f,-0.4f,-1.0f);
  glVertex3f(0.4f,-0.4f,-1.0f);
  glVertex3f(0.8f,0.0f,-1.0f);
}

void FluoGLWidget::receiveColor(const QColor &color)
{
  float r,g,b;
  float a = normalize_0_1(RGB_MAX,RGB_MIN,RGB_MAX);

  qColortoRGB(color,r,g,b);
  updateBackgroundColor(r,g,b,a);
}

void FluoGLWidget::updateBackgroundColor(float r, float g, float b, float a)
{
  makeCurrent();
  glClearColor(r,g,b,a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  update();
}
