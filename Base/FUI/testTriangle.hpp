#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "renderCanvas.hpp"
#include <tuple>
#include <QOpenGLShaderProgram>
#include <QColor>

class TriangleWindow : public RenderCanvas
{

  Q_OBJECT

  public slots:
    void updateBackgroundColor(const QColor& color);

  public:
    TriangleWindow();

    void initialize() override;
    void render() override;

  private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    std::tuple<float,float,float> getRGB(const QColor& color);
    float normalize(float val);


    QOpenGLShaderProgram *m_program;
    int m_frame;

    const char *vertexShaderSource =
        "attribute highp vec4 posAttr;\n"
        "attribute lowp vec4 colAttr;\n"
        "varying lowp vec4 col;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   col = colAttr;\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n";

    const char *fragmentShaderSource =
        "varying lowp vec4 col;\n"
        "void main() {\n"
        "   gl_FragColor = col;\n"
        "}\n";
};

#endif
