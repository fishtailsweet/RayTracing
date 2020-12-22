#ifndef CUBE_H
#define CUBE_H

#include "hittable_list.h"
#include "two_axis_rect.h"

class cube : public hittable  { // 包含6个矩形的轴对齐块图元
    public:
        cube() {}
        cube(const QVector3D &p0, const QVector3D &p1, std::shared_ptr<material> m) : box_min(p0), box_max(p1), mat_ptr(m){
            cal_bounding_box();

            sides.add(std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), m));
            sides.add(std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), m));

            sides.add(std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), m));
            sides.add(std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), m));

            sides.add(std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), m));
            sides.add(std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), m));
            sides.cal_bounding_box();   // 必须有这句话
        };

        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override{
            return sides.hit(r, t_min, t_max, hit_object);
        };
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        void scatter_ray(ray &r) const override{sides.scatter_ray(r);};  // 会被调用到!!!因为它可能被translate或ratote中的指针指到，在这儿卡了好久；但不用判空，因为能进行到这儿说明肯定击中了
        void cal_bounding_box() override {
            box = aabb(box_min, box_max);
        };
        const aabb &get_Bounding_box() const override {return box;}
    private:
        QVector3D box_min, box_max;
        hittable_list sides;
        std::shared_ptr<material> mat_ptr;  // BRDF
        aabb box;
};

#endif // CUBE_H
// 我们需要稍微旋转一下它们，使其与真正的康奈尔盒子匹配。在光线跟踪中，这通常是通过instance完成的，它是已经以某种方式移动或旋转的几何图元。在光线跟踪中，这尤其容易，因为我们什么都不动
// 相反，我们将光线朝相反的方向移动。例如，考虑平移。我们可以在其所有x分量上加上2，或者（就像我们几乎总是在光线跟踪中所做的那样）保留该盒子所在的位置，但在hit函数中，将光线origin的x分量减去2
