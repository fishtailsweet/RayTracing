#ifndef WORLD_H
#define WORLD_H

#include "hittable_list.h"
#include "random_generator.h"

class world : public hittable_list{        // 也是一种hittable_list，只不过是整个世界
    public:
        world() : background_color(0, 0, 0), probability(0.98){}
        world(const QVector3D &back_color, double p, double min, double max) : background_color(back_color), probability(p), t_min(min), t_max(max){}
        void add(std::shared_ptr<hittable> object) override { objects.emplace_back(object); }
        void add_light(std::shared_ptr<hittable> light) { lights.emplace_back(light); }
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override{
            return world_bvh_tree_root->hit(r, t_min, t_max, hit_object);
        };
        void scatter_ray(ray &r) const override{};
        virtual const aabb &get_Bounding_box() const override{return world_bvh_tree_root->get_Bounding_box();}
        QVector3D hit_world(ray &r, bool is_first) const;
        void cal_bounding_box() override {    // 其实是cal_bvh_tree
            if(!objects.empty()){
                world_bvh_tree_root = create_bvh_node(0, objects.size());
            }
        }
        // 该函数是递归的，每条光线都对应一次调用，即在每次调用中hittable_list是不变的但ray是不断变化的，这就是为什么只能将ray作为参数传入hittable_list中，而不能将hittable_list作为参数传入ray的成员函数中
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值，第四个参数是第一个参数对应的光线成功发射出去的概率
    private:
        std::shared_ptr<hittable> create_bvh_node (int start, int end) override{  // 递归地划分所有hittable
            int axis = random_int(0, 2);    // 在每个节点上沿一个轴拆分列表，为了简单起见随机选择一个轴
            auto comparator = (axis == 0) ? hittable_list::box_x_compare
                            : (axis == 1) ? hittable_list::box_y_compare
                                          : hittable_list::box_z_compare;

            int object_span = end - start;
            if (object_span == 1) { // 只可能发生在hittable_world中传入的就是单个元素的src_objects的情况下，即只有场景中只有一个物体时才可能发生
                return objects[start];
            }
            else if (object_span == 2) {        // 单个物体无需用节点包着
                std::shared_ptr<bvh_node> cur_node = std::make_shared<bvh_node>();
                if (comparator(objects[start], objects[start + 1])) {
                    cur_node->set_left_child(objects[start]);
                    cur_node->set_right_child(objects[start + 1]);
                }
                else {
                    cur_node->set_left_child(objects[start + 1]);
                    cur_node->set_right_child(objects[start]);
                }
                cur_node->cal_bounding_box(); // 能执行到这个函数说明此层以下的节点的包围盒都已经计算完了
                return cur_node;
            }
            else {
                auto iterator = objects.begin() + start + object_span / 2;
                std::nth_element(objects.begin() + start, iterator, objects.begin() + end); // 就地操作
                std::shared_ptr<bvh_node> cur_node = std::make_shared<bvh_node>();
                cur_node->set_left_child(create_bvh_node(start, iterator - objects.begin()));
                cur_node->set_right_child(create_bvh_node(iterator - objects.begin(), end));
                cur_node->cal_bounding_box();   // 能执行到这个函数说明此层以下的节点的包围盒都已经计算完了
                return cur_node;
            }
        }
        QVector3D background_color;
        double probability;
        std::vector<std::shared_ptr<hittable>> objects;
        std::shared_ptr<hittable> world_bvh_tree_root;
        std::vector<std::shared_ptr<hittable>> lights;
        double t_min, t_max;
};

#endif // WORLD_H
