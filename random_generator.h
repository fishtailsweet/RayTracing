#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H
#include <QVector3D>
#include <QVector4D>
#include <random>
#include <QtMath>
#include <QDebug>

static double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);   // 必须是static，不然当种子值相同时每次返回的值就都相同了，因为每次都新创建了一个generator且是伪随机
    // distribution是左开右闭
    static std::random_device rd;
    static std::mt19937 generator(rd());
    double ret = distribution(generator);
    return ret;
}
static double random_double(double min, double max) {
    return min + (max - min) * random_double();
}
static QVector3D random_vec3() {
    return QVector3D(random_double(), random_double(), random_double());
}
static QVector3D random_vec3(double min, double max) {
    return QVector3D(random_double(min, max), random_double(min, max), random_double(min, max));
}
static QVector3D random_in_unit_sphere() {  // 单位球内的随机位置，先random_in_unit_sphere再normalize不是半球面上的均匀分布，因为球面不是可展曲面
    QVector3D pos = random_vec3(-1, 1);
    while (true) {  // 舍选法
        if (QVector3D::dotProduct(pos, pos) < 1){   // 在球内
            break;
        }
        pos = random_vec3(-1, 1);
    }
    return pos;
}
static QVector3D random_on_unit_halfsphere_surface() { // 单位半球面上的随机位置，取z轴正半轴为法线方向的上半球，θ是与法线方向形成的立体角，但可看作是在切线空间中的坐标，而切线空间的z轴正是法线
    double ksi1 = random_double();  // 即random_double(0, 1)
    double ksi2 = random_double(0, 2 * M_PI);
    double r = sqrt(1 - pow(ksi1, 2));
    return QVector3D(cos(ksi2) * r, sin(ksi2) * r, ksi1);
    // θ=arccos(ξ_1)，φ=2*Π*ξ_2
    // 单位球面上的随机位置x=sinθcosφ=cos(2*Π*ξ_2)*sqrt(1-ξ_1^2)，y=sinθsinφ=sin(2*Π*ξ_2)*sqrt(1-ξ_1^2)，z=cosθ=ξ_1
}
static QVector3D random_on_unit_sphere_surface(){
    double ksi1 = random_double();  // 即random_double(0, 1)
    double ksi2 = random_double(0, 2 * M_PI);
    double r = 2 * sqrt(ksi1 * (1 - ksi1));
    return QVector3D(cos(ksi2) * r, sin(ksi2) * r, 1 - 2 * ksi1);
}

inline QVector3D random_to_sphere(double radius, double distance_squared) {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = 1 + r2*(sqrt(1-radius*radius/distance_squared) - 1);

    auto phi = 2*M_PI*r1;
    auto x = cos(phi)*sqrt(1-z*z);
    auto y = sin(phi)*sqrt(1-z*z);

    return QVector3D(x, y, z);
}

static QVector3D random_cosine_direction() {
    // 还有一种从二维映射上来的方法见收藏；cosθ加权的单位半球面上的随机位置，取z轴正半轴为法线方向的上半球，θ是与法线方向形成的立体角，但可看作是在切线空间中的坐标，而切线空间的z轴正是法线
    double ksi1 = random_double();  // 即random_double(0, 1)
    double ksi2 = random_double(0, 2 * M_PI);
    double r = sqrt(1 - ksi1);
    return QVector3D(cos(ksi2) * r, sin(ksi2) * r, sqrt(ksi1));
}

static QVector4D random_ggx_direction(double a2) {    // 取z轴正半轴为法线方向；NDF并不是pdf，要乘上(自变量h·宏观法线方向n)归一化后才是，设为cosθ，该pdf的值就是最佳的采样pdf
    double Phi = random_double(0, 2 * M_PI);
    double Theta = random_double();
    double CosTheta = sqrt((1 - Theta) / (1 + (a2 - 1) * Theta));
    double SinTheta = sqrt(1 - CosTheta * CosTheta);
    double d = 1 + CosTheta * CosTheta * (a2 - 1);
    double D = a2 / (M_PI * d * d); // 就是ggx的值即D的值
    //double pdf = D * CosTheta;  // 这里的pdf是立体角意义上的pdf，而不是极坐标意义上的pdf，故不用乘上SinTheta
    double pdf = CosTheta;  // 这里的pdf是立体角意义上的pdf，而不是极坐标意义上的pdf，故不用乘上SinTheta；上下的D约掉，剩下的就是CosTheta
    return QVector4D(SinTheta * cos(Phi), SinTheta * sin(Phi), CosTheta, pdf);
}

static QVector3D random_in_unit_square() {    // 单位圆内的随机位置
    QVector3D pos(random_double(-1, 1), random_double(-1, 1), 0);
    // 为了实现景深模糊，需要在x和y方向进行随机偏移，但z方向不应偏移因为光线的z坐标应是始终不变的即焦距
    while (true) {  // 舍选法
        if (QVector3D::dotProduct(pos, pos) < 1){   // 在圆内
            break;
        }
        pos = QVector3D(random_double(-1, 1), random_double(-1, 1), 0);
    }
    return pos;
}
static int random_int(int min, int max) {
    // Returns a random integer in [min, max]
    return static_cast<int>(random_double(min, max + 1));
}
#endif // RANDOM_GENERATOR_H
