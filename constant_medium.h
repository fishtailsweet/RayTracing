#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "hittable.h"
#include "material.h"
// 可以将其添加到光线跟踪器中的一件事是烟/雾/烟雾。这些称为体积或传播媒介。值得添加的另一个功能是次表面散射，类似于对象内部的浓雾
// 这通常会增加软件体系结构的混乱，因为体积是与表面不同的物体，但是一种简化的技术是使体积成为随机表面
// 体渲染通常的做法是，在体的内部有很多随机表面，来实现散射的效果。比如一束烟可以表示为，在这束烟的内部任意位置，都可以存在一个面，以此来实现烟、雾
// 一堆烟雾可以用一个随机表面取代，该表面可能在该体积的每个点处都可能存在或不存在
// 下面是恒定密度的传播媒介，穿过那里的射线可以在媒介内部散射，也可以笔直穿过。较薄的透明媒介（如小雾）更可能笔直穿过。光线必须在该媒介中传播的距离还决定了光线笔直穿过该媒介的可能性
// 射线穿过媒介时，可能会在任何点散射。媒介越密，则可能性越大；散射的可能性 = 取决于密度的常数 * 需要传播的距离，当反射发生的时候随机获得一个需要传播的距离，若在物体内部需要传播的距离小于这个距离则
class constant_medium : public hittable {
    public:
        constant_medium(std::shared_ptr<hittable> b, double d, std::shared_ptr<texture> a)
            : boundary(b), neg_inv_density(-1 / d), phase_function(std::make_shared<isotropic>(a)){}

        constant_medium(std::shared_ptr<hittable> b, double d, QVector3D c)
            : boundary(b), neg_inv_density(-1 / d), phase_function(std::make_shared<isotropic>(c)){}
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值

        void scatter_ray(ray &r) const override{
            std::shared_ptr<hit_record> record_ptr = r.getRecord(); // 已经与hittable_list中的每个hittable都求过交了，这时的hit_record就是该光线准确的hit_record
            record_ptr->intersection = r.at(record_ptr->t);
            //record_ptr->normal = QVector3D(1, 0, 0);  // 任意的，因为用不到，scatter_ray是随机的，与法线完全没有关系
            //record_ptr->outer2inner = true;     // 也是任意的，因为用不到
            phase_function->scatter(r);
        };

        void cal_bounding_box() override{}; // 不会被调用
        const aabb &get_Bounding_box() const override {return boundary->get_Bounding_box();}
    public:
        std::shared_ptr<hittable> boundary;
        double neg_inv_density; // 烟雾密度
        std::shared_ptr<material> phase_function;
};

#endif // CONSTANT_MEDIUM_H
