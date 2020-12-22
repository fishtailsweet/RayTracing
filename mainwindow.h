#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QVBoxLayout>
#include <QComboBox>

#include "world.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "cube.h"
#include "translate.h"
#include "rotate.h"
#include "flip_face.h"
#include "constant_medium.h"
#include "camera.h"
#include "objloader.h"

#include <qopenglfunctions.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>

class MainWindow : public QOpenGLWidget, protected QOpenGLFunctions // 若不继承QOpenGLFunctions则需添加QOpenGLFunctions *function = context()->functions();，然后用function->调用OpenGL函数
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initializeGL() override;
    void paintGL() override;    // 每帧调用一次，应在它的定义中调用QOpenGLWidget的update()以重绘
    void resizeGL(int w, int h) override;   // 设置OpenGL的视口，投影矩阵等，用于处理窗口大小的变化，以及在第一次显示时会被调用，这是因为新建一个窗口时会自动发送重置窗口大小事件resize event；aspect = w / h
    void rotate_pic(float degree);   // 用于在frag中旋转图片，需要从内存中将对于每个片元值都相同（即uniform）的旋转矩阵传入显存，以避免在shader中重复计算
    void edge_detector(float edge_level, const QVector4D &edge_color, const QVector4D &background_color);
    void gaussian_blur1D(int blur_num, float blur_size, std::unique_ptr<float> gaussian_weight);
    void radial_blur(int blur_num, float blur_size, const QVector2D &center);
    void whirlpool(float rotate_level, const QVector2D &center);   // 漩涡
    void random_scene();   // 生成随机场景
    // “康奈尔盒子”，以模拟光在漫反射表面之间的相互作用。让我们制作5面墙和盒子的灯光
    void cornell_box();
    void final_scene();
    void RGB_split_glitch(float split_level, const QVector2D &split_dir);
    QVector3D ACESToneMapping(QVector3D color, float adapted_lum)
    {
        const float A = 2.51f;
        const float B = 0.03f;
        const float C = 2.43f;
        const float D = 0.59f;
        const float E = 0.14f;

        color *= adapted_lum;
        return (color * (A * color + QVector3D(B, B, B))) / (color * (C * color + QVector3D(D, D, D)) + QVector3D(E, E, E));
    }
protected:
    // void paintEvent (QPaintEvent *e) override;
private slots:
    void onComboBoxSelected(const QString &text);   // connect在槽函数所属的类的成员函数中时该槽函数可以是private，参数个数可以比信号的少但只能从末尾开始省略
private:
    QOpenGLBuffer *m_vbo, *m_uvbo;
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLShaderProgram *m_shader;
    QOpenGLTexture *m_texture;
    world my_world;
    //QVBoxLayout *m_layout;
    //QComboBox *m_combo_box;
};
#endif
