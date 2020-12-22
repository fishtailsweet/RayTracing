#include "two_axis_rect.h"

bool xy_rect::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    double intersection_t = (k - r.getOrigin().z()) / r.getDirection().z();
    if (intersection_t < t_min || intersection_t > t_max){
        return false;
    }
    double x = r.getOrigin().x() + intersection_t * r.getDirection().x();
    double y = r.getOrigin().y() + intersection_t * r.getDirection().y();
    if (x < x0 || x > x1 || y < y0 || y > y1){
        return false;
    }
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->t = intersection_t;
    hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
    return true;
    // 用于在遍历hittable_list时，判断该光线r击中了哪个hittable，击中且交点的t值在范围内时返回true，否则返回false
    // 不在这里设置hit_record的法线和交点坐标及传播光线，而等完全判断好该光线r击中了哪个hittable后再在scatter_ray中进行这些计算，因为这些计算都是很费时的，能少进行最好少进行
}
void xy_rect::scatter_ray(ray &r) const{
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

    record_ptr->u = (temp_intersection.x() - x0) / (x1 - x0);
    record_ptr->v = (temp_intersection.y() - y0) / (y1 - y0);

    QVector3D temp_normal = QVector3D(0, 0, 1);
    if(r.need_reverse_surface()){
        temp_normal = -temp_normal;
    }
    QVector3D always2outer_normal;
    always2outer_normal[0] = cos_theta_y * temp_normal[0] + sin_theta_y * temp_normal[2];  // 正向旋转
    always2outer_normal[1] = temp_normal[1];
    always2outer_normal[2] = -sin_theta_y * temp_normal[0] + cos_theta_y * temp_normal[2];
    record_ptr->set_normal(r.getDirection(), always2outer_normal);   // 第二个参数即交点处的法线方向是始终指向表面外的；/radius以normalize该交点处的法线方向
    // always2outer_normal不仅是单位法线方向，还是是以原点为中心的单位球面上的点
    // 对于球体，纹理坐标通常基于某种形式的经度和纬度，即球形坐标。所以我们计算（θ ，ϕ ） 在球坐标系中 θ 是从底向上的角度（即从-Y向Y），并且 ϕ 是绕Y轴的角度（从-X​​+ Z到+ X-Z再到-X）
    // Windows系统和QPaintDevice中屏幕坐标原点在左上角，但OpenGL中纹理坐标原点在左下角

    //我们要映射 θ 和 ϕ 纹理坐标 ü 和 v 每个在 [0,1]，（u = 0 ，v = 0映射到纹理的左下角）。因此从 （θ ，ϕ ） 至 （u ，v ） 将会：

    mat_ptr->scatter(r);    // 光线传播的计算，其中需要给r指向的hit_record中的光线传播方向scatter_direction和衰减率attenuation赋值
}

bool xz_rect::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    double intersection_t = (k - r.getOrigin().y()) / r.getDirection().y();
    if (intersection_t < t_min || intersection_t > t_max){
        return false;
    }
    double x = r.getOrigin().x() + intersection_t * r.getDirection().x();
    double z = r.getOrigin().z() + intersection_t * r.getDirection().z();
    if (x < x0 || x > x1 || z < z0 || z > z1){
        return false;
    }
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->t = intersection_t;

    hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
    return true;
    // 用于在遍历hittable_list时，判断该光线r击中了哪个hittable，击中且交点的t值在范围内时返回true，否则返回false
    // 不在这里设置hit_record的法线和交点坐标及传播光线，而等完全判断好该光线r击中了哪个hittable后再在scatter_ray中进行这些计算，因为这些计算都是很费时的，能少进行最好少进行
}
void xz_rect::scatter_ray(ray &r) const{
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

    record_ptr->u = (temp_intersection.x() - x0) / (x1 - x0);
    record_ptr->v = (temp_intersection.z() - z0) / (z1 - z0);

    QVector3D temp_normal = QVector3D(0, 1, 0);
    if(r.need_reverse_surface()){
        temp_normal = -temp_normal;
    }
    QVector3D always2outer_normal;
    always2outer_normal[0] = cos_theta_y * temp_normal[0] + sin_theta_y * temp_normal[2];  // 正向旋转
    always2outer_normal[1] = temp_normal[1];
    always2outer_normal[2] = -sin_theta_y * temp_normal[0] + cos_theta_y * temp_normal[2];

    record_ptr->set_normal(r.getDirection(), always2outer_normal);   // 第二个参数即交点处的法线方向是始终指向表面外的；/radius以normalize该交点处的法线方向

    // always2outer_normal不仅是单位法线方向，还是是以原点为中心的单位球面上的点
    // 对于球体，纹理坐标通常基于某种形式的经度和纬度，即球形坐标。所以我们计算（θ ，ϕ ） 在球坐标系中 θ 是从底向上的角度（即从-Y向Y），并且 ϕ 是绕Y轴的角度（从-X​​+ Z到+ X-Z再到-X）
    // Windows系统和QPaintDevice中屏幕坐标原点在左上角，但OpenGL中纹理坐标原点在左下角

    //我们要映射 θ 和 ϕ 纹理坐标 ü 和 v 每个在 [0,1]，（u = 0 ，v = 0映射到纹理的左下角）。因此从 （θ ，ϕ ） 至 （u ，v ） 将会：

    mat_ptr->scatter(r);    // 光线传播的计算，其中需要给r指向的hit_record中的光线传播方向scatter_direction和衰减率attenuation赋值
}

bool yz_rect::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    double intersection_t = (k - r.getOrigin().x()) / r.getDirection().x();
    if (intersection_t < t_min || intersection_t > t_max){
        return false;
    }
    double y = r.getOrigin().y() + intersection_t * r.getDirection().y();
    double z = r.getOrigin().z() + intersection_t * r.getDirection().z();
    if (y < y0 || y > y1 || z < z0 || z > z1){
        return false;
    }
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->t = intersection_t;

    hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);
    return true;
    // 用于在遍历hittable_list时，判断该光线r击中了哪个hittable，击中且交点的t值在范围内时返回true，否则返回false
    // 不在这里设置hit_record的法线和交点坐标及传播光线，而等完全判断好该光线r击中了哪个hittable后再在scatter_ray中进行这些计算，因为这些计算都是很费时的，能少进行最好少进行
}
void yz_rect::scatter_ray(ray &r) const{
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

    record_ptr->v = (temp_intersection.y() - y0) / (y1 - y0);
    record_ptr->u = (temp_intersection.z() - z0) / (z1 - z0);

    QVector3D temp_normal = QVector3D(1, 0, 0);
    if(r.need_reverse_surface()){
        temp_normal = -temp_normal;
    }
    QVector3D always2outer_normal;
    always2outer_normal[0] = cos_theta_y * temp_normal[0] + sin_theta_y * temp_normal[2];  // 正向旋转
    always2outer_normal[1] = temp_normal[1];
    always2outer_normal[2] = -sin_theta_y * temp_normal[0] + cos_theta_y * temp_normal[2];

    record_ptr->set_normal(r.getDirection(), always2outer_normal);   // 第二个参数即交点处的法线方向是始终指向表面外的；/radius以normalize该交点处的法线方向
    // always2outer_normal不仅是单位法线方向，还是是以原点为中心的单位球面上的点
    // 对于球体，纹理坐标通常基于某种形式的经度和纬度，即球形坐标。所以我们计算（θ ，ϕ ） 在球坐标系中 θ 是从底向上的角度（即从-Y向Y），并且 ϕ 是绕Y轴的角度（从-X​​+ Z到+ X-Z再到-X）
    // Windows系统和QPaintDevice中屏幕坐标原点在左上角，但OpenGL中纹理坐标原点在左下角

    //我们要映射 θ 和 ϕ 纹理坐标 ü 和 v 每个在 [0,1]，（u = 0 ，v = 0映射到纹理的左下角）。因此从 （θ ，ϕ ） 至 （u ，v ） 将会：

    mat_ptr->scatter(r);    // 光线传播的计算，其中需要给r指向的hit_record中的光线传播方向scatter_direction和衰减率attenuation赋值
}
