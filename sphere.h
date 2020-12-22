#ifndef SPHERE_H
#define SPHERE_H
#include "hittable.h"
#include "material.h"
class sphere : public hittable {
    public:
        sphere() {}
        sphere(const QVector3D &cen, double r, std::shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {cal_bounding_box();};
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        void scatter_ray(ray &r) const override;
        void cal_bounding_box() override;
        const aabb &get_Bounding_box() const override {return box;}
        // 当我们从球体外部的某个点均匀地采样一个球体的立体角时，我们实际上只是在均匀地采样一个圆锥体（圆锥体与球体相切）
        double pdf_value(const ray &r, const QVector3D &to_light_vec) const override {
            std::shared_ptr<hit_record> record_ptr = r.getRecord(); // 已经与hittable_list中的每个hittable都求过交了，这时的hit_record就是该光线准确的hit_record
            double cos_theta_max = sqrt(1 - radius * radius / (center - record_ptr->intersection).lengthSquared());
            double solid_angle = 2 * M_PI * (1 - cos_theta_max);
            return  1 / solid_angle;
         }
         QVector3D random(const QVector3D &origin) const override {
             QVector3D direction = center - origin;
             auto distance_squared = direction.lengthSquared();
             onb uvw;
             uvw.build_from_normal(direction);
             return uvw.convert_to_tangent_space(random_to_sphere(radius, distance_squared));
         }
    private:
        QVector3D center;   // 球心坐标
        double radius;  // 半径
        std::shared_ptr<material> mat_ptr;  // BRDF
        aabb box;
};

#endif
