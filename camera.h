#ifndef CAMERA_H
#define CAMERA_H
#include "ray.h"
#include "random_generator.h"

class camera {  // 可以制作更酷的相机
    public:
        camera(double width, double height, double vertical_fov_degree, const QVector3D &pos, const QVector3D &lookat_point, const QVector3D &vertical_up,
               double aperture_diam, int sample_num, double _time0 = 0, double _time1 = 0) :
            camera_pos(pos), aperture_radius(aperture_diam / 2), samples_per_pixel(sample_num), time0(_time0), time1(_time1){
            // vertical_up是指定的世界空间中垂直向上的向量（不必须是(0, 1, 0)），而非相机的垂直向上向量，因为相机可能倾斜，这类似公告板技术Billboarding；它的作用是构建相机空间即类似onb中的切线空间
            // pos是相机位置，lookat_point是相机看向的点，因为是左手坐标系，故pos->lookat_point是一个轴
            QVector3D lookat_vector = lookat_point - pos;   // 为了得到左手坐标系（ue4和unity都是左手，OpenGL是右手），相机空间的z轴应朝向虚拟屏幕；左右手坐标系的互相转化只需将z坐标取反即可
            double focus_dist = lookat_vector.length();  // 焦距（单位向量的长度定为1）
            double radian = M_PI * vertical_fov_degree / 180;  // 传入的vertical_fov的单位是度，1弧度=(180/π)度，1度=(π/180)弧度
            // <QtMath>自带M_PI和角度转弧度float qDegreesToRadians(float degrees)；内置三角函数用的都是弧度
            double viewport_height = 2 * (focus_dist * qTan(radian / 2));  // 虚拟屏幕的宽（单位向量的长度定为1）
            double viewport_width = viewport_height * width / height; // 虚拟屏幕的高根据实际屏幕的aspect_ratio和虚拟屏幕的宽计算出来（单位向量的长度定为1）
            // 必须在这儿现算aspect_ratio = width / height而不能将aspect_ratio算好后作为实参传入，不然浮点数精度误差会导致渲染出来的是椭圆
            unit_lookat_vector = lookat_vector.normalized();    // 和normalize()不同，normalize()是就地修改，而normalized()是返回一个normalize后的新向量
            unit_horizontal = QVector3D::crossProduct(unit_lookat_vector, vertical_up);
            unit_horizontal.normalize();
            unit_vertical = QVector3D::crossProduct(unit_lookat_vector, unit_horizontal);  // 不用normalize了，因为叉乘的两个向量都是单位向量且一定互相垂直（即夹角为90度）
            horizontal =  viewport_width * unit_horizontal;   // 相机空间的x轴，正方向为右，长度为虚拟屏幕的宽
            vertical = -viewport_height * unit_vertical;    // 相机空间的y轴，正方向为上，长度为虚拟屏幕的高；取负是为了得到左手坐标系，因为叉乘的结果是右手坐标系
            lower_left_corner = camera_pos + lookat_vector - horizontal / 2 + vertical / 2; // 由相机指向虚拟屏幕的左上角，因为QPaintDevice的原点默认为QPaintDevice的左上角
        }
        ray get_ray(double u, double v) const {
            QVector3D dof_vector = aperture_radius * random_in_unit_sphere(); // 光圈半径越大，景深depth of field模糊越严重
            QVector3D offset = unit_horizontal * dof_vector.x() + unit_vertical * dof_vector.y();
            return ray(camera_pos + offset, (lower_left_corner + u * horizontal - v * vertical - camera_pos - offset).normalized(), random_double(time0, time1)); // 因为是从虚拟屏幕的左上角开始发射射线的
        // 因为所有光线均来自camera_pos，故原始相机是一个针孔相机即没有景深模糊，屏幕实际上就是相机的传感器
        // 为了模拟景深模糊，需要将起点在x和y方向随机偏移，但z方向不应偏移因为光线的z坐标应是始终不变的即焦距
        // 距相机越远的物体，打到它的光线的传播时间相对越长，故景深模糊的随机偏移累积越严重
        // 射出的每条光线都要指定一个射出时间，可以通过在快门打开时的某个随机时间发送每条光线来获得随机估计
        }
        int get_samples_num() const {
            return samples_per_pixel;
        }
    private:
        QVector3D camera_pos;   // 相机位置
        QVector3D lower_left_corner;    // 由相机指向虚拟屏幕的左上角，因为QPaintDevice的原点默认为QPaintDevice的左上角
        QVector3D horizontal;   // 相机空间的x轴，正方向为右，长度为虚拟屏幕的宽（单位向量的长度定为1）
        QVector3D vertical; // 相机空间的y轴，正方向为上，长度为虚拟屏幕的高（单位向量的长度定为1）
        QVector3D unit_horizontal, unit_vertical, unit_lookat_vector;   // 依次是相机空间的x，y，z轴的单位向量
        double aperture_radius; // 光圈半径；光圈半径越大，景深模糊越严重
        int samples_per_pixel;  // 向虚拟屏幕的每个像素发射的光线的个数，即采样次数
        double time0, time1;  // 快门开关的时间，用于模拟运动模糊，在真实的相机中，快门会打开并保持打开状态一段时间，相机和物体可能会在这段时间内移动。它实际上是相机在我们想要的时间间隔内看到的图像的平均值
};
#endif
