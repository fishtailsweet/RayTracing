#ifndef XY_RECT_H
#define XY_RECT_H
#include "hittable.h"
#include "material.h"

// 矩形通常方便于对人造环境进行建模。我喜欢做轴对齐的矩形，因为它们很容易。（我们将实例化，以便稍后再旋转它们。）
// 这是xy平面中的矩形。这样的平面由（z = k \）,轴线对齐的矩形由线\（x = x_0 \），\（x = x_1 \），\（y = y_0 \）和\（y = y_1 \）定义
class xy_rect : public hittable {   // 不需要box，故自己也可以作为aabb
    public:
        xy_rect() {}

        xy_rect(double _x0, double _x1, double _y0, double _y1, double _k,
            std::shared_ptr<material> mat)
            : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mat_ptr(mat) {cal_bounding_box();};

        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        void scatter_ray(ray &r) const override;
        const aabb &get_Bounding_box() const override {return box;}
        /*因为我们的矩形是轴对齐的，所以它们的边界框将具有无限薄的一面。将它们与我们的轴向对齐的边界体积层次结构分开时，可能会出现问题
        为了解决这个问题，所有可击中的对象都应获得一个边界框，该边界框在每个维度上的宽度都是有限的。对于我们的矩形，我们只需要在无边框的那一侧稍微垫上盒子即可*/
        void cal_bounding_box() override {
            // The bounding box must have non-zero width in each dimension, so pad the Z
            // dimension a small amount.
            box = aabb(QVector3D(x0, y0, k - 0.0001), QVector3D(x1, y1, k + 0.0001));
        }
    private:
        double x0, x1, y0, y1, k;   // z = k
        std::shared_ptr<material> mat_ptr;  // BRDF
        aabb box;
};


class xz_rect : public hittable {   // 不需要box，故自己也可以作为aabb
    public:
        xz_rect() {}

        xz_rect(double _x0, double _x1, double _z0, double _z1, double _k,
            std::shared_ptr<material> m)
            : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mat_ptr(m) {cal_bounding_box();};

        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        void scatter_ray(ray &r) const override;
        const aabb &get_Bounding_box() const override {return box;}
        /*因为我们的矩形是轴对齐的，所以它们的边界框将具有无限薄的一面。将它们与我们的轴向对齐的边界体积层次结构分开时，可能会出现问题
        为了解决这个问题，所有可击中的对象都应获得一个边界框，该边界框在每个维度上的宽度都是有限的。对于我们的矩形，我们只需要在无边框的那一侧稍微垫上盒子即可*/
        void cal_bounding_box() override {
            // The bounding box must have non-zero width in each dimension, so pad the Z
            // dimension a small amount.
            box = aabb(QVector3D(x0, k - 0.0001, z0), QVector3D(x1, k + 0.0001, z1));
        }
        double pdf_value(const ray &r, const QVector3D &to_light_vec) const override {
            auto area = (x1-x0)*(z1-z0);
            std::shared_ptr<hit_record> record_ptr = r.getRecord(); // 已经与hittable_list中的每个hittable都求过交了，这时的hit_record就是该光线准确的hit_record
            auto distance_squared = to_light_vec.lengthSquared();
            auto cosine = fabs(QVector3D::dotProduct(to_light_vec.normalized(), record_ptr->normal));
            // record_ptr->normal始终指向入射光线起始点所在方向，这样在计算reflect、refract和对光源采样时可以直接使用
            return distance_squared / (cosine * area);
            // dw = dAcosθ' / (光源上的该采样点 - 着色点)的模的平方，而采样dw和采样dA的概率应相等即pdf_向光源发射光线 * dw = (1/A) * dA，故得pdf_向光源发射光线 = (dA / dw) / A = (光源上的该采样点 - 着色点)的模的平方 / (cosθ' * A)；
         }
         QVector3D random(const QVector3D &origin) const override {
            QVector3D temp = QVector3D(random_double(x0,x1), k, random_double(z0,z1));
            return temp - origin;
         }
    private:
        double x0, x1, z0, z1, k;   // y = k
        std::shared_ptr<material> mat_ptr;  // BRDF
        aabb box;
};

class yz_rect : public hittable {   // 不需要box，故自己也可以作为aabb
    public:
        yz_rect() {}

        yz_rect(double _y0, double _y1, double _z0, double _z1, double _k,
            std::shared_ptr<material> m)
            : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mat_ptr(m) {cal_bounding_box();};

        bool hit(ray &r, double t_min, double t_max, const hittable *&hit_object) const override;
        // 第二个参数是第一个参数对应的光线可以击中的最小t值，第三个参数是第一个参数对应的光线可以击中的最大t值
        void scatter_ray(ray &r) const override;
        const aabb &get_Bounding_box() const override {return box;}
        /*因为我们的矩形是轴对齐的，所以它们的边界框将具有无限薄的一面。将它们与我们的轴向对齐的边界体积层次结构分开时，可能会出现问题
        为了解决这个问题，所有可击中的对象都应获得一个边界框，该边界框在每个维度上的宽度都是有限的。对于我们的矩形，我们只需要在无边框的那一侧稍微垫上盒子即可*/
        void cal_bounding_box() override {
            // The bounding box must have non-zero width in each dimension, so pad the Z
            // dimension a small amount.
            box = aabb(QVector3D(k - 0.0001, y0, z0), QVector3D(k + 0.0001, y1, z1));
        }
    private:
        double y0, y1, z0, z1, k;   // x = k
        std::shared_ptr<material> mat_ptr;  // BRDF
        aabb box;
};

#endif // XY_RECT_H
