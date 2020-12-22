#include "constant_medium.h"

bool constant_medium::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const
{
    std::shared_ptr<hit_record> record_ptr = r.getRecord(); // 已经与hittable_list中的每个hittable都求过交了，这时的hit_record就是该光线准确的hit_record
    double init_temp_t = record_ptr->t;
        if (!boundary->hit(r, -INFINITY, INFINITY, hit_object)){  // 背面也算
            return false;
        }
        double first_t = record_ptr->t;
        if (!boundary->hit(r, first_t + 0.001, INFINITY, hit_object)){
            record_ptr->t = init_temp_t;
            return false;
        }

        if (first_t < t_min){
            first_t = t_min;
        }
        if (record_ptr->t > t_max){
            record_ptr->t = t_max;
        }

        if (first_t >= record_ptr->t){
            record_ptr->t = init_temp_t;
            return false;
        }
        if (first_t < 0){       // 防止雾里套雾
            first_t = 0;
        }
        const auto ray_length = r.getDirection().length();
        const auto distance_inside_boundary = (record_ptr->t - first_t) * ray_length;
        const auto hit_distance = neg_inv_density * log(random_double());
        if (distance_inside_boundary < hit_distance){

            record_ptr->t = init_temp_t;
            return false;
        }
        // 当光线通过体积时，它可能在任何点散射s

        record_ptr->t = first_t + hit_distance / ray_length;

        hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
        return true;
}
//我们必须对边界周围的逻辑如此谨慎的原因是，我们需要确保它对起点在媒介内的射线有效。在云层中，事物会大量反弹，因此这是常见的情况
// 以上代码假定光线一旦离开恒定的介质边界，它将永远在边界之外继续传播。换句话说，它假定边界形状是凸的。因此，此特定实现将适用于像盒子或球体这样的边界，但不适用于包含空隙的圆环或形状
// 可以编写处理任意形状的实现，但是我们将其留给读者练习
