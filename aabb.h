#ifndef AABB_H
#define AABB_H

#include "ray.h"

class aabb {    // 包围盒
    public:
        aabb() {}
        aabb(const QVector3D &min, const QVector3D &max) : minimum(min), maximum(max){}
        aabb(const aabb &temp) : minimum(temp.minimum), maximum(temp.maximum){}
        void operator=(const aabb &temp){
            minimum = temp.minimum;
            maximum = temp.maximum;
        }

        QVector3D getMin() const {return minimum; }
        QVector3D getMax() const {return maximum; }

        bool hit(const ray& r, double t_min, double t_max) const {  // 判断光线是否与包围盒相交的效率较高的方法
            for (int i = 0; i < 3; ++i) {
                double invD = 1.0 / r.getDirection()[i];
                double t0 = (minimum[i] - r.getOrigin()[i]) * invD;
                double t1 = (maximum[i] - r.getOrigin()[i]) * invD;
                if (invD < 0){
                    std::swap(t0, t1);
                }
                t_min = t0 > t_min ? t0 : t_min;
                t_max = t1 < t_max ? t1 : t_max;
                if (t_max <= t_min){
                    return false;
                }
            }
            return true;
        }
        static aabb surrounding_box(const aabb &box0, const aabb &box1) { // 用于将两个包围盒合并为一个包围盒
            QVector3D small(fmin(box0.minimum.x(), box1.minimum.x()),
                         fmin(box0.minimum.y(), box1.minimum.y()),
                         fmin(box0.minimum.z(), box1.minimum.z()));

            QVector3D big(fmax(box0.maximum.x(), box1.maximum.x()),
                       fmax(box0.maximum.y(), box1.maximum.y()),
                       fmax(box0.maximum.z(), box1.maximum.z()));

            return aabb(small,big);
        }
    private:
        QVector3D minimum;
        QVector3D maximum;
};

#endif // AABB_H
