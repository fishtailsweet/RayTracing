#ifndef ONB_H
#define ONB_H
#include <QVector3D>
class onb { // 切线空间
    public:
        onb() {}

        QVector3D operator[](int i) const { return axis[i]; }

        QVector3D Bitangent() const { return axis[0]; } // 从法矢，即副切线，x轴
        QVector3D Tangent() const { return axis[1]; } // 切线，y轴
        QVector3D Normal() const { return axis[2]; } // 法线，z轴

        QVector3D convert_to_tangent_space(double x, double y, double z) const {
            return x * axis[0] + y * axis[1] + z * axis[2];
        }

        QVector3D convert_to_tangent_space(const QVector3D &vec) const {    // 将传入的向量看作是在切线空间中的坐标
            return vec.x() * axis[0] + vec.y() * axis[1] + vec.z() * axis[2];
        }

        void build_from_normal(const QVector3D &n){ // 创建切线空间
            axis[2] = n.normalized();   // 法线
            QVector3D temp = (abs(axis[2].x()) > 0.9) ? QVector3D(0, 1, 0) : QVector3D(1, 0, 0);   // 避免这个任意选择的temp与法向量平行
            axis[1] = QVector3D::crossProduct(axis[2], temp).normalized();  // 切线
            axis[0] = QVector3D::crossProduct(axis[2], axis[1]);    // 从法矢，即副切线
        };
    private:
        QVector3D axis[3];
};
#endif // ONB_H
