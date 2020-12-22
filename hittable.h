#ifndef HITTABLE_H
#define HITTABLE_H
#include "ray.h"
#include "aabb.h"

class hittable {    // 为光线可能击中的任何事物创建一个抽象类
public:
    virtual bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const = 0;     // 用于在遍历hittable_list时，判断该光线r击中了哪个hittable
    // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值，击中且交点的t值在范围内时返回true，否则返回false；第四个参数必须是指针引用
    // 最后一个参数之所以必须是指向const的指针引用是因为this指针自带顶层const
    virtual void scatter_ray(ray &r) const = 0;
    // 通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
    //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
    virtual void cal_bounding_box() = 0;
    //现在，我们需要添加一个函数来计算所有命中表的边界框。然后，我们将在所有图元上形成一个框的层次结构，并且各个图元（例如球体）将生活在叶子上。该函数返回布尔值，因为并非所有基本体都具有边界框（例如，无限平面）
    virtual const aabb &get_Bounding_box() const = 0;
    virtual double pdf_value(const ray &r, const QVector3D &to_light_vec) const {}   // 其实这两个新函数都应是纯虚的，用于将这个物体用作灯光时调用
    virtual QVector3D random(const QVector3D &origin) const {}
};

#endif
//光线与物体的交点是光线跟踪器中的主要时间瓶颈，并且时间与物体的数量成线性关系。但这是在相同模型上的重复搜索，因此我们应该能够本着二进制搜索的精神使它成为对数搜索。因为我们要在同一模型上发送数百万至数十亿条射线，
//所以我们可以对模型进行分类，然后每个射线相交可以是一个亚线性搜索。排序的两个最常见的族是：1）划分空间，和2）划分对象。后者通常更容易编写代码，并且对于大多数模型来说运行起来也一样快
// 在一组图元上限制体积的关键思想是找到一个完全包围（包围）所有对象的体积。例如，假设您计算了10个对象的边界球。错过边界球的任何射线肯定会错过所有十个对象。如果射线撞击边界球，则可能会撞击十个对象之一
// 关键是我们将对象划分为子集。我们不分割空间。任何对象都只有一个边界体积，但是边界体积可以重叠
// 为了使事物亚线性，我们需要使边界体积分层。例如，如果我们将一组对象分为红色和蓝色两组，并使用矩形边界体积，请注意，蓝色和红色的边界体积包含在紫色的边界体积中，但是它们可能重叠，并且没有顺序-它们都在内部。因此，树没有左右孩子的顺序概念。他们只是在里面。该代码将是：
/*if (hits purple)
    hit0 = hits blue enclosed objects
    hit1 = hits red enclosed objects
    if (hit0 or hit1)
        return true and info of closer hit
return false*/
// 需要一种将光线与边界体积相交的方法。光线边界体积的交点需要快速，边界体积必须非常紧凑。实际上，对于大多数模型来说，AABB比其他盒子更好用，但是，如果遇到异常类型的模型，则始终要牢记这一设计选择
// 我们只需要知道是否击中它就可以了。我们不需要生命值或法线，也不需要我们要显示的对象所需的任何东西
// 以下伪代码确定 Ť 楼板中的间隔重叠：
/*compute (tx0, tx1)
compute (ty0, ty1)
compute (tz0, tz1)
return overlap?( (tx0, tx1), (ty0, ty1), (tz0, tz1))*/
// 有一些注意事项使它不如最初出现的那么漂亮。首先，假设射线在负方向传播X方向。间隔(Ťx 0,ŤX 1)按上述方法计算可能会相反，如像(7,3)。其次，那里的鸿沟可能会给我们带来无限性。并且，如果射线原点位于平板边界之一上，则可以得到NaN。在各种光线追踪器的AABB中，有许多方法可以解决这些问题
// （还有诸如SIMD之类的矢量化问题，我们将不在此处讨论。如果您想在矢量化方面多加努力以提高速度）就我们而言，这不太可能成为主要瓶颈只要我们使它合理地快，那么我们就简单点吧，这通常是最快的！
// 一件麻烦的事是，完全有效的光线将具有 bX=0，导致被零除。这些射线有些在平板内部，有些不在。同样，零在IEEE浮点下将具有±号。的好消息bX=0 就是它 Ťx 0 和 ŤX 1 如果不在两者之间，则均为+∞或均为-∞ X0 和 X1。因此，使用min和max应该可以为我们提供正确的答案：
// 如果我们这样做，剩下的麻烦事就是 bx=0 还有 x0−Ax=0 或 X1−Ax=0所以我们得到一个NaN。在那种情况下，我们可能会接受命中答案，也可能不接受命中答案，但是我们稍后会再讨论。
// 现在，让我们看一下重叠功能。假设我们假设间隔没有反转（因此第一个值小于间隔中的第二个值），并且在这种情况下我们想返回true。布尔重叠，也可以计算对于间隔 （d，D ） 和 （e，E) 的重叠间隔（f，F) ：
/*bool overlap(d, D, e, E, f, F)
    f = max(d, e)
    F = min(D, E)
    return (f < F)*/
//如果周围有任何NaN运行，则比较将返回false，因此如果我们关心放牧的情况，我们需要确保边界框有一点填充（而且我们可能应该这样做，因为最终在ray tracer中会出现所有情况）。所有三个维度都在一个循环中，并在间隔中传递[t_min, t_max]
