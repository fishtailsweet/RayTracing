#include "moving_sphere.h"

QVector3D moving_sphere::cal_center(double time) const {    // 创建线性都有其中心迁移从一个球体类 center0的time0来center1处time1
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}
bool moving_sphere::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {
    QVector3D oc = r.getOrigin() - cal_center(r.getTime());  // 由球心指向光线起始点
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
void moving_sphere::scatter_ray(ray &r) const{
    // 通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
    //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->intersection = r.at(record_ptr->t); // t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的

    QVector3D translate_intersection = record_ptr->intersection - r.get_offset();

    double radian_angle_y = qDegreesToRadians(r.get_degree_angle_y());
    double sin_theta_y = sin(radian_angle_y);
    double cos_theta_y = cos(radian_angle_y);

    QVector3D temp_intersection;
    temp_intersection[0] = cos_theta_y * translate_intersection[0] - sin_theta_y * translate_intersection[2];  // 正向旋转
    temp_intersection[1] = translate_intersection[1];
    temp_intersection[2] = sin_theta_y * translate_intersection[0] + cos_theta_y * translate_intersection[2];

    QVector3D temp_normal = (temp_intersection - cal_center(r.getTime())) / radius;
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

    mat_ptr->scatter(r);    // 光线传播的计算，其中需要给r指向的hit_record中的光线传播方向scatter_direction和衰减率attenuation赋值
}
void moving_sphere::cal_bounding_box() {
    aabb box0(cal_center(time0) - QVector3D(radius, radius, radius),
        cal_center(time0) + QVector3D(radius, radius, radius));
    aabb box1(cal_center(time1) - QVector3D(radius, radius, radius),
        cal_center(time1) + QVector3D(radius, radius, radius));
    box = aabb::surrounding_box(box0, box1);
}
