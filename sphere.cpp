#include "sphere.h"

bool sphere::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    // 用于在遍历hittable_list时，判断该光线r击中了哪个hittable，击中且交点的t值在范围内时返回true，否则返回false
    QVector3D oc = r.getOrigin() - center;  // 由球心指向光线起始点
    double a = QVector3D::dotProduct(r.getDirection(), r.getDirection());
    double half_b = QVector3D::dotProduct(oc, r.getDirection());
    double c = QVector3D::dotProduct(oc, oc) - pow(radius, 2);
    double delta = half_b * half_b - a * c;    // 矢量形式的球面方程|(P-center)|^2 = (P-center)点乘(P-center) = radius^2与矢量形式的射线方程P = origin + t*direction联立得到的关于t的一元二次方程的Δ
    // 简化后的Δ和交点公式；点乘满足分配律和结合律
    if (delta > 0) {    // 有交点
        double temp = sqrt(delta);
        double root = (-half_b - temp) / a; // 近处交点
        if (root > t_min && root < t_max) { // 说明近处交点的t值在范围内
            std::shared_ptr<hit_record> record_ptr = r.getRecord();
            record_ptr->t = root;
            hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
            // 不在这里设置hit_record的法线和交点坐标及传播光线，而等完全判断好该光线r击中了哪个hittable后再在scatter_ray中进行这些计算，因为这些计算都是很费时的，能少进行最好少进行
            return true;
        }
        root = (-half_b + temp) / a;    // 近处交点<=t_min时，远处交点才可能被相交到
        if (root > t_min && root < t_max) {
            std::shared_ptr<hit_record> record_ptr = r.getRecord();
            record_ptr->t = root;
            hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
            // 不在这里设置hit_record的法线和交点坐标及传播光线，而等完全判断好该光线r击中了哪个hittable后再在scatter_ray中进行这些计算，因为这些计算都是很费时的，能少进行最好少进行
            return true;
        }
    }
    return false;
}
void sphere::scatter_ray(ray &r) const{
    // 通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
    //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->intersection = r.at(record_ptr->t); // t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的

    QVector3D translate_intersection = record_ptr->intersection - r.get_offset();

    double radian_angle_y = qDegreesToRadians(r.get_degree_angle_y());
    double sin_theta_y = sin(radian_angle_y);
    double cos_theta_y = cos(radian_angle_y);

    QVector3D temp_intersection;
    temp_intersection[0] = cos_theta_y * translate_intersection[0] - sin_theta_y * translate_intersection[2];  // 反向旋转
    temp_intersection[1] = translate_intersection[1];
    temp_intersection[2] = sin_theta_y * translate_intersection[0] + cos_theta_y * translate_intersection[2];

    QVector3D temp_normal = (temp_intersection - center) / radius;
    if(r.need_reverse_surface()){
        temp_normal = -temp_normal;
    }
    QVector3D always2outer_normal;
    always2outer_normal[0] =  cos_theta_y * temp_normal[0] + sin_theta_y * temp_normal[2];  // 正向旋转
    always2outer_normal[1] = temp_normal[1];
    always2outer_normal[2] = -sin_theta_y * temp_normal[0] + cos_theta_y * temp_normal[2];

    record_ptr->set_normal(r.getDirection(), always2outer_normal);   // 第二个参数即交点处的法线方向是始终指向表面外的；/radius以normalize该交点处的法线方向

    double theta = acos(-always2outer_normal.y());
    double phi = atan2(-always2outer_normal.z(), always2outer_normal.x()) + M_PI;
    record_ptr->u = phi / (2 * M_PI);
    record_ptr->v = theta / M_PI;

    /*QVector3D translate_intersection = record_ptr->intersection + r.get_offset();

    record_ptr->intersection[0] = cos_theta_y * translate_intersection[0] + sin_theta_y * translate_intersection[2];  // 正向旋转
    record_ptr->intersection[1] = translate_intersection[1];
    record_ptr->intersection[2] = -sin_theta_y * translate_intersection[0] + cos_theta_y * translate_intersection[2];*/

    // always2outer_normal不仅是单位法线方向，还是是以原点为中心的单位球面上的点
    // 对于球体，纹理坐标通常基于某种形式的经度和纬度，即球形坐标。所以我们计算（θ ，ϕ ） 在球坐标系中 θ 是从底向上的角度（即从-Y向Y），并且 ϕ 是绕Y轴的角度（从-X​​+ Z到+ X-Z再到-X）
    // Windows系统和QPaintDevice中屏幕坐标原点在左上角，但OpenGL中纹理坐标原点在左下角
    /*我们要映射 θ 和 ϕ 纹理坐标 ü 和 v 每个在 [0,1]，（u = 0 ，v = 0映射到纹理的左下角）。因此从 （θ ，ϕ ） 至 （u ，v ） 将会：
    u =ϕ/2π，v =θ/π
    计算 θ 和 ϕ 对于以原点为中心的单位球面上的给定点，我们从对应的笛卡尔坐标的方程式开始：
    y=-cosθ，x=-cosφsinθ，z=sinφsinθ
    我们需要将这些方程式求逆。由于<cmath> 函数atan2()，它接受与正弦和余弦成比例的任意一对数字并返回角度，我们可以传递X和z求出 ϕ:
    ϕ = atan2(z，-x)
    atan2()返回-π至π范围内的值，但它们从0到π，然后从π瞬间翻转到-π到0。虽然这在数学上是正确的，但我们想要u范围从 0 至 1，而不是 0到1/2 然后从1/2瞬间翻转到 −1/2到0
    atan2(a, b) = atan2(-a, -b) + π,
    上面的第二种公式得出的值是连续的0到2π。因此，我们可以计算 ϕ 如
    ϕ = atan2(-z, x) + π（因为atan2(z，-x) = atan2(-z, x)）
    θ的推导更简单：
    θ = acos(-y)
    该函数获取以原点为中心的单位球面上的点，并进行计算 u和v*/
    // p: 以原点为中心的单位球面上的点
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    mat_ptr->scatter(r);    // 光线传播的计算，其中需要给r指向的hit_record中的光线传播方向scatter_direction和衰减率attenuation赋值
}
void sphere::cal_bounding_box() {
    box = aabb(
        center - QVector3D(radius, radius, radius),
        center + QVector3D(radius, radius, radius));
}
