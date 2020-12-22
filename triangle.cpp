#include "triangle.h"

bool triangle::hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const {    // 函数定义若不在函数声明内，就一定要单独放在cpp文件中，而不能放在h文件中，否则会链接错误
    QVector3D E1 = vertex1 - vertex0;
    QVector3D E2 = vertex2 - vertex0;
    QVector3D P = QVector3D::crossProduct(r.getDirection(), E2);
    double det = QVector3D::dotProduct(E1, P);  // 行列式
    // keep det > 0, modify T accordingly
    QVector3D T;
    if(det > 0){
        T = r.getOrigin() - vertex0;
    }
    else{
        T = vertex0 - r.getOrigin();
        det = -det;
    }
    // If determinant is near zero, ray lies in plane of triangle
    if(det < 0.0001){
        return false;
    }
    // Calculate u and make sure u <= 1
    double u = QVector3D::dotProduct(T, P);
    if(u < 0.0 || u > det){
        return false;
    }
    QVector3D Q = QVector3D::crossProduct(T, E1);
    // Calculate v and make sure u + v <= 1
    double v = QVector3D::dotProduct(r.getDirection(), Q);
    if(v < 0.0 || u + v > det){
         return false;
    }
    // Calculate t, scale parameters, ray intersects triangle
    double t = QVector3D::dotProduct(E2, Q);
    double fInvDet = 1.0 / det;
    t *= fInvDet;
    if(t < t_min || t > t_max){
        return false;
    }
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->t = t;
    record_ptr->u = u * fInvDet;   // obj文件中的三角形和别的图形不一样，在这儿正好顺手计算了，它的uv的意义也跟别的uv的意义不相同，即这里代表重心坐标（重心坐标是一个凸组合）
    record_ptr->v = v * fInvDet;

    //record_ptr->intersection = r.at(intersection_t); // t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的
    hit_object = this;// 这里绝对不能写hit_object = std::make_shared<hittable>(this);

    return true;
}
void triangle::scatter_ray(ray &r) const{
    // 通过遍历hittable_list，在每个hittalble的hit中完全判断好该光线r击中了哪个hittable后，再在该hittable的scatter_ray中只进行一次r指向的hit_record中法线normal、交点intersection坐标
    //（t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的）和光线传播scatter的计算
    std::shared_ptr<hit_record> record_ptr = r.getRecord();
    record_ptr->intersection = r.at(record_ptr->t); // t已经在hit中计算完并存好了，二者都需要，因为t的值是由光线的direction的值来决定的，而direction的值是可能被光线的normalize_direction()改变的
    double u = record_ptr->u;
    double v = record_ptr->v;

    record_ptr->u = (1 - u - v) * uv0.x() + u * uv1.x() + v * uv2.x();  // 到这儿，uv的意义才是正常的纹理坐标的uv的意义
    record_ptr->v = (1 - u - v) * uv0.y() + u * uv1.y() + v * uv2.y();

    QVector3D temp_normal = (1 - u - v) * normal0 + u * normal1 + v * normal2;
    temp_normal.normalize();
    if(r.need_reverse_surface()){
        temp_normal = -temp_normal;
    }
    double radian_angle_y = qDegreesToRadians(r.get_degree_angle_y());
    double sin_theta_y = sin(radian_angle_y);
    double cos_theta_y = cos(radian_angle_y);

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
