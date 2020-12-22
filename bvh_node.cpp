#include <algorithm>
#include "bvh_node.h"
// 如果您发送的是像没有边界框的无限平面之类的东西，则检查是否有边界框。我们没有任何这些原语，因此在添加此类东西之前不应该发生
// 任何效率结构（包括BVH）中最复杂的部分就是构建它。我们在构造函数中执行此操作。关于BVH的一个很酷的事情是，只要将a中的对象列表bvh_node 分为两个子列表，hit函数就会起作用
// 如果划分得好，将是最好的方法，这样两个孩子的边界框要比其父母的边界框小，但这是为了提高速度而不是正确性。我将选择中间位置，并在每个节点上沿一个轴拆分列表。我会为了简单起见：
// 1.
// 2.排序原语（using std::sort）
// 3.在每个子树中放一半
// 当进入的列表是两个元素时，我在每个子树中放置一个，然后结束递归。遍历算法应该是平稳的，不必检查空指针，因此，如果我只有一个元素，则可以在每个子树中复制它
// 显式检查三个元素并仅执行一次递归可能会有所帮助，但我认为整个方法稍后将进行优化。这样产生：
bool bvh_node::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {
    if (!box.hit(r, t_min, t_max)){ // 没击中包围盒就不可能击中其中的物体
        return false;
    }
    bool hit_left = left_child->hit(r, t_min, t_max, hit_object);
    if(right_child == left_child){  // 说明这次判断的节点中只有一个物体
        return hit_left;
    }
    bool hit_right = right_child->hit(r, t_min, hit_left ? r.getRecord()->t : t_max, hit_object);   // 别忘了更新t_max
    return hit_left || hit_right;
}
