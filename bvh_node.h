#ifndef BVH_NODE_H
#define BVH_NODE_H

#include "hittable.h"
#include "random_generator.h"

class bvh_node : public hittable {
    public:
        bvh_node(){};   // 必须有default构造函数，不然就无法创建空的hittable_list和world，因为hittable_list和world中有bvh_node成员
        bvh_node(const bvh_node &temp) : left_child(temp.left_child), right_child(temp.right_child), box(temp.box){}
        void operator=(const bvh_node &temp){
            left_child = temp.left_child;
            right_child = temp.right_child;
            box = temp.box;
        }
        bool hit(ray& r, double t_min, double t_max, const hittable *&hit_object) const override;

        void cal_bounding_box() override{
            if(left_child == right_child){
                box = left_child->get_Bounding_box();
            }
            else{
                box = aabb::surrounding_box(left_child->get_Bounding_box(), right_child->get_Bounding_box());   // 能执行到这儿时说明在此层以下的单一包围盒都已经计算完了
            }
        }   // 能执行到这个函数说明此层以下的节点的包围盒都已经计算完了
        const aabb &get_Bounding_box() const override {return box;}

        void scatter_ray(ray &r) const override{};  // scatter_ray是纯虚函数故必须有，虽然它不可能被调用

        std::shared_ptr<hittable> get_left_child(){return left_child;}
        std::shared_ptr<hittable> get_right_child(){return right_child;}

        void set_left_child(std::shared_ptr<hittable> left){left_child = left;}
        void set_right_child(std::shared_ptr<hittable> right){right_child = right;}
    private:    // 成员函数指针和普通函数指针不同，存储的是偏移量，故应使用static成员函数；因为只在这个类中被使用故声明为private即可
        std::shared_ptr<hittable> left_child;
        std::shared_ptr<hittable> right_child;
        aabb box;
};

#endif // BVH_NODE_H
