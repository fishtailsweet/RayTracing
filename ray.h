#ifndef RAY_H
#define RAY_H   // 头文件引用的最顶层，即谁也不引用
#include <QVector3D>
#include <memory>
#include <QtMath>
#include <QDebug>
struct hit_record {
    enum state : unsigned{
        diffuse = 0,
        emittied = 1,
        specular_or_refract_isotropic = 2,
    };
    double t {INFINITY};   // 交点对应的t，必须初始化为INFINITY以与world.hit_world中赋给t_max参数的INFINITY相呼应，否则会出错，在hittable的hit中被赋值
    double u, v;    // 交点对应的纹理坐标
    QVector3D normal;   // 交点处的法线方向，始终指向入射光线起始点所在方向，这样在计算reflect、refract和对光源采样时可以直接使用，在hittable的scatter_ray中被赋值
    // 这样在外界就无法通过 入射光线的方向向量与该法线方向的点积的符号 来确定入射光线在曲面的哪一侧因为符号一定<0，故必须用inner2outer存储该信息，这对于两面效果不同的物体（如双面的纸）或具有内部和外部的物体（如玻璃球）很重要
    bool outer2inner;    // 在hittable的scatter_ray中被赋值，为true说明该光线从表面外射向表面内
    QVector3D BRDF_multiply_cos_or_emmited;  // 衰减率，在material的scatter中被赋值
    QVector3D scatter_direction;    // 光线传播方向，即要发射的下一条光线的方向向量，在material的scatter中被赋值
    double scatter_pdf;
    QVector3D intersection; // 在hittable的scatter_ray中被赋值，必须有这个而不能只用at(record_ptr->t)表示交点，因为在normalize_direction后，该光线指向的hit_record中t的值就不再可靠了，因为t的值是由光线的direction的值来决定的
    inline void set_normal(const QVector3D &direct, const QVector3D &always2outer_normal) { // 第一个参数是入射光线的方向向量，第二个参数是始终指向表面外的交点处的法线方向
        outer2inner = QVector3D::dotProduct(direct, always2outer_normal) < 0;   // 为true说明该光线从表面外射向表面内
        normal = outer2inner ? always2outer_normal : -always2outer_normal;  // 使法线方向始终指向入射光线起始点所在方向，这样在计算reflect、refract和对光源采样时可以直接使用
    }
    state cur_state;
};

class ray {
public:
    ray() {}
    ray(const QVector3D &orig, const QVector3D &dir, double launch_time = 0) : origin(orig), direction(dir), record_ptr(std::make_shared<hit_record>()), time(launch_time), degree_angle_y(0), offset(QVector3D(0, 0, 0)), is_front_surface(true) {}
    // 一定一定一定记得给每个成员变量赋初值，不然它就可以是任意数，不一定是0的！！！
    // 别忘了给sin和cos赋初始值
    ray(const ray &r) : origin(r.origin), direction(r.direction), record_ptr(r.record_ptr), time(r.time), degree_angle_y(r.degree_angle_y), offset(r.offset), is_front_surface(r.is_front_surface) {}
    void operator=(const ray &r){
        origin = r.origin;
        direction = r.direction;
        record_ptr = r.record_ptr;
        time = r.time;
        degree_angle_y = r.degree_angle_y;
        offset = r.offset;
        is_front_surface = r.is_front_surface;
    }

    const QVector3D &getOrigin() const { return origin; }   // 现代c++的get函数都是返回const引用的，注意const不能省

    void setOrigin(const QVector3D &new_origin){origin = new_origin;}
    void setDirection(const QVector3D &new_direction){direction = new_direction;}

    const QVector3D &getDirection() const { return direction; }
    QVector3D at(double t) const {
        return origin + t * direction;
    }
    std::shared_ptr<hit_record> getRecord() const {return record_ptr;}
    double getTime() const { return time; }
    // void normalize_direction(){direction.normalize();}   // 不能有这个函数，因为我需要方向向量的长度
    // 注意在normalize_direction后，该光线指向的hit_record中t的值就不再可靠了，而只有intersection是可靠的，因为t的值是由光线的direction的值来决定的

    double get_degree_angle_y() const { return degree_angle_y; }
    void set_degree_angle_y(double theta_y) { degree_angle_y = theta_y; }
    const QVector3D &get_offset() const { return offset; }
    void set_offset(const QVector3D &displacement) { offset = displacement; }
    void reverse_surface(){is_front_surface = !is_front_surface;}
    bool need_reverse_surface() const{return !is_front_surface;}
private:
    QVector3D origin; // 光线起始点
    QVector3D direction;  // 光线的方向向量，不一定是单位向量，注意在normalize_direction后，该光线指向的hit_record中t的值就不再可靠了，而只有intersection是可靠的，因为t的值是由光线的direction的值来决定的
    std::shared_ptr<hit_record> record_ptr;
    double time;  // 射出时间
    double degree_angle_y;
    QVector3D offset;
    bool is_front_surface;
};

#endif
