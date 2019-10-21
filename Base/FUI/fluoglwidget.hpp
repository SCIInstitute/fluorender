#ifndef FLUOGLWIDGET_HPP
#define FLUOGLWIDGET_HPP

#include <QOpenGLWindow>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMainWindow>
#include <QColor>

//class RenderCanvas

//class FluoGLWidget : public QOpenGLWidget, public QOpenGLFunctions
class FluoGLWidget : public QOpenGLWindow, public QOpenGLFunctions
{
  Q_OBJECT

  public slots:
    void receiveColor(const QColor& color);

  public:
    FluoGLWidget(QWidget *parent = nullptr, bool ifMain = false);
    FluoGLWidget(bool ifMain) : FluoGLWidget(nullptr,ifMain){}

  protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

  private:
    void qColortoRGB(const QColor &C, float &r, float &g, float &b) const;
    void renderTriangle(float r, float g, float b);
    void renderHexagon(float r, float g, float b);
    float normalize_0_1(float val, float min, float max) const;

    void updateBackgroundColor(float r, float g, float b, float a);

    bool isMain;
    const float RGB_MIN = 1.0f;
    const float RGB_MAX = 255.0f;
};

#endif // FLUOGLWIDGET_HPP
