#ifndef FLIP_FACE_H
#define FLIP_FACE_H
#include "hittable.h"

class flip_face : public hittable { // 天花板上的灯光周围发出嘈杂的声音，是因为灯光是双面的，并且灯光和天花板之间的空间很小
    public:
        flip_face(std::shared_ptr<hittable> p) : ptr(p) {}

        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override{
            if(ptr->hit(r, t_min, t_max, hit_object)){
                hit_object = this;
                return true;
            }
            return false;
        }

        void scatter_ray(ray &r) const override{
            // 将光线方向反向就相当于将物体表面反向是行不通的，因为要求在交点处的法线方向必须始终指向入射光线起始点所在方向，而若将光线方向反向了则不好判断了
            r.reverse_surface();
            ptr->scatter_ray(r);
            r.reverse_surface();              // 一定记得要反向回来，不然之后就会出错
        }
        // 通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
        //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
        void cal_bounding_box() override{}
        //现在，我们需要添加一个函数来计算所有命中表的边界框。然后，我们将在所有图元上形成一个框的层次结构，并且各个图元（例如球体）将生活在叶子上。该函数返回布尔值，因为并非所有基本体都具有边界框（例如，无限平面）
        const aabb &get_Bounding_box() const override{
            return ptr->get_Bounding_box();
        }
        double pdf_value(const ray &r, const QVector3D &to_light_vec) const override {
            return ptr->pdf_value(r, to_light_vec);
         }
        QVector3D random(const QVector3D &origin) const override {
           return ptr->random(origin);
        }
    private:
        std::shared_ptr<hittable> ptr;
};

#endif // FLIP_FACE_H
