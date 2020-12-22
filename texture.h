#ifndef TEXTURE_H
#define TEXTURE_H
#include <QVector3D>
#include <memory>
#include <QtMath>
#include <QImage>
#include "perlin.h"

class texture { // 恒定纹理
    public:
        virtual QVector3D value(double u, double v, const QVector3D &intersection) const = 0;
};

class solid_color : public texture {
    public:
        solid_color() {}
        solid_color(const QVector3D &c) : color_value(c) {}

        solid_color(double red, double green, double blue)
          : solid_color(QVector3D(red, green, blue)) {}

        QVector3D value(double u, double v, const QVector3D &intersection) const override {
            return color_value;
        }

    private:
        QVector3D color_value;
};

// 可以通过注意到正弦和余弦的符号只是以规则的方式交替来创建一个棋盘格纹理，并且如果我们在所有三个维度上乘以三角函数，
// 则该乘积的符号会形成3D棋盘格图案
// 这些检查器的奇/偶指针可以是恒定的纹理，也可以是其他一些过程纹理
class checker_texture : public texture {
    public:
        checker_texture() {}

        checker_texture(std::shared_ptr<texture> _even, std::shared_ptr<texture> _odd)
            : odd(_odd), even(_even) {}

        checker_texture(const QVector3D &c1, const QVector3D &c2)
            : odd(std::make_shared<solid_color>(c2)), even(std::make_shared<solid_color>(c1)) {}

        QVector3D value(double u, double v, const QVector3D &intersection) const override {
            double sines = sin(10 * intersection.x()) * sin(10 * intersection.y()) * sin(10 * intersection.z());
            if (sines < 0){
                return odd->value(u, v, intersection);
            }
            return even->value(u, v, intersection);
        }

    public:
        std::shared_ptr<texture> odd;
        std::shared_ptr<texture> even;
};

// 创建一个实际的纹理，并使用这些介于0和1之间的浮点并创建灰色（即rgb分量相等），则：
class noise_texture : public texture {
    public:
        noise_texture() {}
        // 它的频率也有点低。我们可以缩放输入点以使其变化更快：
        noise_texture(double sc) : scale(sc) {}
        QVector3D value(double u, double v, const QVector3D &intersection) const override {
            //return QVector3D(1, 1, 1) * noise.turb(scale * p);
            //但是，通常会间接使用湍流。大理石状纹理。基本思想是使颜色与正弦函数等成正比，
            //并使用湍流来调整相位（因此它会移动x在 sin（x）），使条纹起伏。注释掉直接的噪音和湍流，并给出类似大理石的效果是：
            return QVector3D(1, 1, 1) * 0.5 * (1 + sin(scale * intersection.z() + 10 * noise.turb(intersection)));
        }

    public:
        perlin noise;
        double scale;   // 频率，频率越高变化越明显
};
// 保存图像的纹理类，每个分量的范围是[0,255]
class image_texture : public texture {
    public:
        image_texture(){}

        image_texture(const char *filename) : image(filename){}

        QVector3D value(double u, double v, const QVector3D &intersection) const override {
            u = qBound(0.0, u, 1.0);    // qBound等价于clamp
            v = 1.0 - qBound(0.0, v, 1.0);  // 不这么做会上下颠倒
            QColor color = image.pixelColor(static_cast<int>(u * (image.width() - 1)), static_cast<int>(v * (image.height() - 1)));
            // QColor会自动将每个变量都除以255，很狗的；我们的程序中统一用[0, 1]的颜色，故这里需要/255，到最后mainwindow中再乘一个255
            return QVector3D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0); // 每个分量都属于[0, 1]
        }

    private:
        QImage image;
};

#endif // TEXTURE_H
