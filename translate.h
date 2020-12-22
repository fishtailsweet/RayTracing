#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "hittable.h"
// 将其视为移动还是坐标更改取决于您。用于移动任何基础命中表的代码是一个转换实例
class translate : public hittable {
    public:
        translate(std::shared_ptr<hittable> ptr, const QVector3D &displacement)
            : hittable_ptr(ptr), offset(displacement) {cal_bounding_box();}
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override{
            // 不能在这儿修改传入光线的origin，因为并不一定光线就是打到了这个hittable，只有完全确定光线确实打到了这个hittable后，才能在传入光线本身上修改它的信息，即应在scatter_ray中修改而不是hit中
            // 但是不在这儿修改的话，即传入的不是r的话，r的hit_record中的t就没法被赋值了；能进行到这儿说明它以下的东西都是被平移的，故可以先修改，改完之后再改回来
            //ray moved_r(r.getOrigin() - offset, r.getDirection(), r.getTime());
            r.setOrigin(r.getOrigin() - offset);
            if (hittable_ptr->hit(r, t_min, t_max, hit_object)){
                hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);这句话绝对不能落，因为我们要回到这里来传播
                r.setOrigin(r.getOrigin() + offset);
                return true;
            }
            r.setOrigin(r.getOrigin() + offset);
            return false;
        }
        void scatter_ray(ray &r) const override{
            // 完全确定了光线确实打到了这个hittable，这时就可以随便在传入光线本身上修改它的信息，即应在scatter_ray中修改而不是hit中
            r.set_offset(r.get_offset() + offset);
            hittable_ptr->scatter_ray(r);
            // 平移时物体表面的法线向量不会变，所以简单，定义都不用
        }
        void cal_bounding_box() override{
            box = aabb(hittable_ptr->get_Bounding_box().getMin() + offset, hittable_ptr->get_Bounding_box().getMax() + offset);
        }   // 变换的包围盒只是为了它可以被光线击中，故包围盒是正常变换的，而其中的物体因为千奇百怪故不选择变换物体，而选择变换光线
        const aabb &get_Bounding_box() const override {return box;}
    private:
        std::shared_ptr<hittable> hittable_ptr;
        QVector3D offset;
        aabb box;
};

#endif // TRANSLATE_H
