#include "world.h"

// 场景中唯一的光来自发射器。为此，我们将背景色参数添加到ray_color函数中，并注意新emitted值
QVector3D world::hit_world(ray &r, bool is_first_or_previous_is_not_diffuse) const { // 该函数是递归的，每条光线都对应一次调用
    if(!objects.empty()){
        const hittable *hit_object = nullptr;    // 没const不行的；必须是指向const的指针是因为this指针自带顶层const
        if(hit(r, t_min, t_max, hit_object)){
            hit_object->scatter_ray(r);// 完全判断好哪个hittable被击中后再在scatter_ray中进行对hit_record中法线normal、交点intersection及纹理uv的坐标（不是t的计算，二者都需要）和光线传播scatter的计算，因为这些计算都是很费时的，能少进行最好少进行
            std::shared_ptr<hit_record> record_ptr = r.getRecord(); // 已经与hittable_list中的每个hittable都求过交了，这时的hit_record就是该光线准确的hit_record

            if(record_ptr->cur_state == hit_record::emittied) { // 直接光照已经计算过了
                if(is_first_or_previous_is_not_diffuse){   // 如果第一条光线直接就打到了灯上，那还是要算的
                    return record_ptr->BRDF_multiply_cos_or_emmited;
                }
                return QVector3D(0, 0, 0);
            }
            QVector3D dir_light = QVector3D(0, 0, 0);
            if(record_ptr->cur_state == hit_record::diffuse){
                for(std::shared_ptr<hittable> light : lights){
                    QVector3D random_direction = light->random(record_ptr->intersection).normalized();
                    ray r_to_light(record_ptr->intersection, random_direction, r.getTime());    // 在hit_object->scatter_ray()中对这里需要的hit_record的成员已经赋值好了；多了最后一个参数，因为光的传播认为是瞬时的，故射出时间在传播时不会改变
                    const hittable *temp_hit_object = nullptr;    // 没const不行的；必须是指向const的指针是因为this指针自带顶层const
                    hit(r_to_light, t_min, t_max, temp_hit_object);
                    if(temp_hit_object == light.get()){
                        temp_hit_object->scatter_ray(r_to_light);
                        double pdf = light->pdf_value(r, r_to_light.getRecord()->intersection - r_to_light.getOrigin());
                        dir_light += (r_to_light.getRecord()->BRDF_multiply_cos_or_emmited * record_ptr->BRDF_multiply_cos_or_emmited / pdf);
                    }
                }
            }
            // r必须是非const左值引用，因为需要将各种信息记录在指向的hit_record中
            if (random_double() > probability) {    // 俄罗斯轮盘赌RR
                return dir_light;      // 这次调用对应的光线没有发射出去
            }
            ray r_next(record_ptr->intersection, record_ptr->scatter_direction, r.getTime());    // 在hit_object->scatter_ray()中对这里需要的hit_record的成员已经赋值好了；多了最后一个参数，因为光的传播认为是瞬时的，故射出时间在传播时不会改变
            if(record_ptr->cur_state == hit_record::specular_or_refract_isotropic){
                return dir_light + hit_world(r_next, true) * record_ptr->BRDF_multiply_cos_or_emmited / (record_ptr->scatter_pdf * probability);    // QVector3D之间的乘法是对应分量直接相乘，有一部分光被吸收；除以probability是为了保证期望是无偏估计
            }
            return dir_light + hit_world(r_next, false) * record_ptr->BRDF_multiply_cos_or_emmited / (record_ptr->scatter_pdf * probability);    // QVector3D之间的乘法是对应分量直接相乘，有一部分光被吸收；除以probability是为了保证期望是无偏估计
        }
        /*r.normalize_direction();    // normalize光线的方向向量，这时hit_record中的t就没法用了因为方向向量发生了变化，这就是为什么hit_record中需要存储交点intersection的坐标
        auto t = 0.5 * (r.getDirection().y() + 1.0);    // 从normalize的[-1, 1]映射到[0, 1]，可视化单位向量的经典方法
        return (1.0 - t) * QVector3D(1.0, 1.0, 1.0) + t * QVector3D(0.5, 0.7, 1.0);*/   // 模拟天空，白和蓝lerp
        return background_color;
    }
    return background_color;
}
// 对于列表，您可以在构造时存储边界框，也可以动态计算边界框。我喜欢动态，因为通常只在BVH施工中才使用它
// PDF的任何加权平均值都是PDF，但混合反射光线和直接光照光线有些微妙，因为它们的pdf会有重合的方向，可能反射方向恰好就是直接光照方向
