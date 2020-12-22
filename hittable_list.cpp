#include "hittable_list.h"

bool hittable_list::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    //double cur_t_max = t_max;   // 因为需要将这次调用对应的光线与hittable_list中的每个hittable求交，故与每个hittable求交后都需要更新好当前可以击中的最大t值，不需要保存最小t值是因为只有最大t值可能被更新
    // 必须得这么创建并作为参数传入hit，不然不知道最终打到的是哪个了，不能在hit_record中存储因为循环引用了；aabb中无需赋值，只有真正的物体上才需对该参数赋值
    // 之所以是const是为了被赋成this，因为this自带顶层引用
    // 保存好最终击中的是哪个hittable，这样通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
    //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
    /*double cur_t_max = t_max;
    for (auto object : objects) {    // 需要将这次调用对应的光线与hittable_list中的每个hittable求交，这次调用对应的光线指向的hit_record的成员t在最后的值为交点中最小的t
        if (object->hit(r, t_min, cur_t_max, hit_object)) {  // 与每个hittable的子类求交，以给这次调用对应的光线指向的hit_record赋值
            if(r.getRecord()->t != cur_t_max) { // 说明这次求交发生了更新，一定在t值更小的点处击中了object
                cur_t_max = r.getRecord()->t;   // 更新当前可以击中的最大t值
                hit_object = object.get();    // 更新当前击中的是哪个hittable
            }
        }
    }
    if(cur_t_max == t_max) { // 相等说明没击中任何hittable
        return false;
    }
    return true;*/
    if(!objects.empty()){
        bool is_hit = world_bvh_tree_root->hit(r, t_min, t_max, hit_object);
        if(is_hit){
            the_hit_object = hit_object;
        }
        return is_hit;
    }
    return false;
}
