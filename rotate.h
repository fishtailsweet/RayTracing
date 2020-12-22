#ifndef ROTATE_H
#define ROTATE_H

#include "hittable.h"
class rotate_y : public hittable {  // 绕y轴旋转
    public:
        rotate_y(std::shared_ptr<hittable> ptr, double theta) : hittable_ptr(ptr), degree_angle(theta){
            cal_bounding_box();
        }
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        void scatter_ray(ray &r) const override{

            r.set_degree_angle_y(r.get_degree_angle_y() + degree_angle);

            // 完全确定了光线确实打到了这个hittable，这时就可以随便在传入光线本身上修改它的信息，即应在scatter_ray中修改而不是hit中
            hittable_ptr->scatter_ray(r);
            // uv不用重新赋，但法线、交点和下一条光线的方向都需要重新计算（交点不用了，t在旋转前后是相同的）
        }
        // 与平移情况不同，表面法线向量也会发生变化，因此如果受到打击，我们也需要变换方向。幸运的是，旋转适用相同的公式。如果添加比例，事情将变得更加复杂
        void cal_bounding_box() override;
        const aabb &get_Bounding_box() const override {return box;}
    private:
        std::shared_ptr<hittable> hittable_ptr;
        double degree_angle;
        mutable QVector3D initial_origin, initial_direction;    // 只能这样，不然也不能因为你一个坏了所有的hit
        aabb box;
};

#endif // ROTATE_H
