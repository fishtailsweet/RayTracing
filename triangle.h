#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "hittable.h"
#include "material.h"
#include <QVector2D>

class triangle : public hittable {   // 不需要box，故自己也可以作为aabb
       public:
           triangle() {}

           triangle(const QVector3D &first_vertex, const QVector3D &second_vertex, const QVector3D &third_vertex,
                    const QVector3D &first_normal, const QVector3D &second_normal, const QVector3D &third_normal,
                    const QVector2D &first_uv, const QVector2D &second_uv, const QVector2D &third_uv,
                    std::shared_ptr<material> mat)
               : vertex0(first_vertex), vertex1(second_vertex), vertex2(third_vertex),
               normal0(first_normal), normal1(second_normal), normal2(third_normal),
               uv0(first_uv), uv1(second_uv), uv2(third_uv),
               mat_ptr(mat) {cal_bounding_box();};

           bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
           // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
           void scatter_ray(ray &r) const override;
           const aabb &get_Bounding_box() const override {return box;}
           /*因为我们的矩形是轴对齐的，所以它们的边界框将具有无限薄的一面。将它们与我们的轴向对齐的边界体积层次结构分开时，可能会出现问题
           为了解决这个问题，所有可击中的对象都应获得一个边界框，该边界框在每个维度上的宽度都是有限的。对于我们的矩形，我们只需要在无边框的那一侧稍微垫上盒子即可*/
           void cal_bounding_box() override {
               box = aabb(QVector3D(std::fmin(std::fmin(vertex0.x(), vertex1.x()), vertex2.x()), std::fmin(std::fmin(vertex0.y(), vertex1.y()), vertex2.y()), std::fmin(std::fmin(vertex0.z(), vertex1.z()), vertex2.z())),
                          QVector3D(std::fmax(std::fmax(vertex0.x(), vertex1.x()), vertex2.x()), std::fmax(std::fmax(vertex0.y(), vertex1.y()), vertex2.y()), std::fmax(std::fmax(vertex0.z(), vertex1.z()), vertex2.z())));

           }
       private:
           QVector3D vertex0, vertex1, vertex2;
           QVector3D normal0, normal1, normal2;
           QVector2D uv0, uv1, uv2;
           std::shared_ptr<material> mat_ptr;  // BRDF
           aabb box;
};

#endif // TRIANGLE_H
