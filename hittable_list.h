#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H
#include "hittable.h"
#include "bvh_node.h"

class hittable_list : public hittable {    // 为光线可能击中的任何事物创建一个抽象类；可以作为hittable被传入world中
    public:
        hittable_list(){}
        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        virtual void add(std::shared_ptr<hittable> object) { objects.emplace_back(object);}
        void scatter_ray(ray &r) const override{
            the_hit_object->scatter_ray(r);
        }
        void cal_bounding_box() override{   // 其实是cal_bvh_tree
            if(!objects.empty()){
                world_bvh_tree_root = create_bvh_node(0, objects.size());
            }
        };
        const aabb &get_Bounding_box() const override{return world_bvh_tree_root->get_Bounding_box();}
    protected :
        virtual std::shared_ptr<hittable> create_bvh_node(int start, int end){  // 递归地划分所有hittable
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
                std::nth_element(objects.begin() + start, iterator, objects.begin() + end, comparator); // 就地操作；一个划分操作
                std::shared_ptr<bvh_node> cur_node = std::make_shared<bvh_node>();
                cur_node->set_left_child(create_bvh_node(start, iterator - objects.begin()));
                cur_node->set_right_child(create_bvh_node(iterator - objects.begin(), end));
                cur_node->cal_bounding_box();   // 能执行到这个函数说明此层以下的节点的包围盒都已经计算完了
                return cur_node;
            }
        }
        static bool box_compare(const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b, int axis) {
            return a->get_Bounding_box().getMin()[axis] < b->get_Bounding_box().getMin()[axis];
        }

        static bool box_x_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
            return hittable_list::box_compare(a, b, 0);
        }

        static bool box_y_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
            return hittable_list::box_compare(a, b, 1);
        }

        static bool box_z_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
            return hittable_list::box_compare(a, b, 2);
        }
    private :
        std::vector<std::shared_ptr<hittable>> objects;
        mutable const hittable *the_hit_object;
        // 必须有const才能被赋值，而因为hit_list是const故必须加mutable，之所以hit_list必须是const是因为hittable如cube的hit是const，const成员函数中只能调用成员的const成员函数
        std::shared_ptr<hittable> world_bvh_tree_root;
};


#endif // HITTABLE_LIST_H
