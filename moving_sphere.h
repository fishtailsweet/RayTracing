#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H
#include "hittable.h"
#include "material.h"

class moving_sphere : public hittable { // 几乎与sphere完全相同，只是真正的center坐标会变而已
    public:
        moving_sphere() {cal_bounding_box();}
        moving_sphere(const QVector3D &cen0, const QVector3D &cen1, double r, std::shared_ptr<material> m, double _time0, double _time1)
              : center0(cen0), center1(cen1), radius(r), mat_ptr(m), time0(_time0), time1(_time1) {cal_bounding_box();};
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        void scatter_ray(ray &r) const override;
        QVector3D cal_center(double time) const;    // 使它们全部运动，而固定球的起点和终点位置
        void cal_bounding_box() override;
        const aabb &get_Bounding_box() const override {return box;}
private:
    QVector3D center0, center1;   // 球心坐标
    double radius;  // 半径
    std::shared_ptr<material> mat_ptr;  // BRDF
    double time0, time1;
    aabb box;
};

#endif // MOVING_SPHERE_H
