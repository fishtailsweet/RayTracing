#ifndef MATERIAL_H
#define MATERIAL_H
#include "texture.h"
#include "random_generator.h"
#include "ray.h"
#include "onb.h"

// 可以使用具有很多参数的通用material，不同种material只是将其中一些参数归零，也可以像这里一样使用抽象的material类
// 一些光线被物体吸收absorption了，而另一些光线被散射scattering到其他方向，散射分为反射和折射，散射到物体内部称为折射refraction或透射transmission，
// 折射进入物体内部的光线还会继续与内部的颗粒相交，其中一些光线最后会重新发射出物体表面即漫反射的一部分（将具有和入射光线不同的方向分布和颜色），而另一些则被物体吸收；散射到物体外部称为反射reflection
class material {    // 即BRDF
public:
    virtual void scatter(ray &r_in) const = 0;   // 光线传播的计算，其中需要给r_in指向的hit_record中的光线传播方向scatter_direction、衰减率attenuation和emmited赋值
    QVector3D reflect(const QVector3D &unit_direction, const QVector3D &unit_normal) const{ // 镜面反射方向计算
        // 该函数必须是const成员函数，不然scatter中没法调用它，因为scatter是const成员函数
        // 传入的两个向量必须都是单位向量，计算结果才对
        return unit_direction - 2 * QVector3D::dotProduct(unit_direction, unit_normal) * unit_normal;
    }
    QVector3D refract(const QVector3D &unit_direction, const QVector3D &unit_normal, double refraction_ratio) const{    // 折射方向计算
        // 该函数必须是const成员函数，不然scatter中没法调用它，因为scatter是const成员函数
        // 传入的两个向量必须都是单位向量，计算结果才对
        // 折射满足Snell定律η1sinθ1 = η2sinθ2，传入的第三个参数是 sin(折射角)/sin(入射角) 即 入射介质的折射率/出射介质的折射率
        double cos_theta = QVector3D::dotProduct(-unit_direction, unit_normal);
        QVector3D perp = refraction_ratio * (unit_direction + unit_normal * cos_theta); // 折射向量的垂直于法线的分量
        QVector3D parallel = -sqrt(fabs(1.0 - QVector3D::dotProduct(perp, perp))) * unit_normal;    // 折射向量的平行于法线的分量
        return perp + parallel; // 垂直分量+水平分量
    }
    double SchlickGGX(double NdotV, double k) const{    // 千万别忘了const
        return NdotV / (NdotV * (1 - k) + k);
    }
    QVector3D SchlickFresnel(const QVector3D &F0, double NdotVReflect) const{
        return F0 + (QVector3D(1, 1, 1) - F0) * pow(1 - NdotVReflect, 5);
    }
};

class lambertian : public material {    // diffuse哑光材质，lambert漫反射
public:
    lambertian(const QVector3D &a) : albedo(std::make_shared<solid_color>(a)) {}
    lambertian(std::shared_ptr<texture> a) : albedo(a) {}

    virtual void scatter(ray &r_in) const override {
        // 千万别忘了const，否则就没有重写该成员函数，因为this指针形参自带顶层const，成员函数为const时附带底层const（只有底层const对特征标有影响，顶层const则没有，而只有指针/引用才可能有底层const）
        std::shared_ptr<hit_record> record_ptr = r_in.getRecord();
        //record_ptr->scatter_direction = record_ptr->normal + random_on_unit_halfsphere_surface();   // 可以这样得到cosθ加权的半球采样，也可以直接求random_cosine_direction()（见图片）
        onb uvw;
        uvw.build_from_normal(record_ptr->normal);  // 交点处的法线方向，始终指向入射光线起始点所在方向
        record_ptr->scatter_direction = uvw.convert_to_tangent_space(random_cosine_direction());   // 将传入的向量看作是在切线空间中的坐标；得到的一定是单位向量故不用normalize
        // 在半球上接近法线方向漫反射概率较高，远离法线方向即接近掠射角方向漫反射概率较低
        // random_on_unit_halfsphere_surface是半球面上的均匀分布（pdf是cos(θ)/派），但先random_in_unit_sphere再normalize则不是半球面上的均匀分布（pdf是cos^3(θ)/派，更远离法线）
        QVector3D BRDF = albedo->value(record_ptr->u, record_ptr->v, record_ptr->intersection) / M_PI;   // 将该material的反射率赋给光线的衰减率
        double cosine = QVector3D::dotProduct(record_ptr->normal, record_ptr->scatter_direction);
        record_ptr->BRDF_multiply_cos_or_emmited = BRDF * cosine;    // 内积中的法线必须是单位向量

        record_ptr->cur_state = hit_record::diffuse;

        record_ptr->scatter_pdf = cosine <= 0 ? 0.0001 : cosine / M_PI;    // 内积中的法线必须是单位向量；注意pdf要在积分域上归一化；采样漫反射用的pdf就是brdf的归一化；要作为分母故不能为0
        // 给r_in的record中的pdf赋值，代表该方向的概率
    }

private:
    //QVector3D albedo;   // 反射率即光没被吸收的概率，随表面颜色的变化而不同，要赋给每条打到该material上的光线指向的hit_record中的衰减率attenuation成员
    std::shared_ptr<texture> albedo;    // 通过用const color& a纹理指针代替来制作纹理材料
    // 可以将任何一种纹理分配给Lambertian材质，Lambertian不需要意识到这一点
};

class metal : public material { // 金属，镜面反射
public:
    metal(const QVector3D &a, double r, double m) : albedo(a), roughness(r < 1 ? r : 1), metalness(m) {}
    virtual void scatter(ray &r_in) const override {
        std::shared_ptr<hit_record> record_ptr = r_in.getRecord();
        onb uvw;
        uvw.build_from_normal(record_ptr->normal);  // 交点处的法线方向，始终指向入射光线起始点所在方向
        double aerfa = roughness * roughness;   // 或α = ((roughness + 1) / 2)^2
        QVector4D actual_normal_and_pdf = random_ggx_direction(aerfa * aerfa);
        QVector3D actual_normal = uvw.convert_to_tangent_space(QVector3D(actual_normal_and_pdf.x(), actual_normal_and_pdf.y(), actual_normal_and_pdf.z()));   // 将传入的向量看作是在切线空间中的坐标；得到的一定是单位向量故不用normalize
        // record_ptr->normal始终指向入射光线起始点所在方向，这样在计算reflect、refract和对光源采样时可以直接使用
        record_ptr->scatter_direction = reflect(r_in.getDirection().normalized(), actual_normal); // 传入的两个向量必须都是单位向量，计算结果才对
        double cosine = QVector3D::dotProduct(actual_normal, record_ptr->scatter_direction);
        double k = aerfa * aerfa / 2;    // 直接光照，IBL是 (α + 1)^2 / 8，α = roughness^2
        QVector3D F0 = metalness * albedo + (1 - metalness) * QVector3D(0.04, 0.04, 0.04);
        record_ptr->BRDF_multiply_cos_or_emmited = SchlickGGX(QVector3D::dotProduct(-r_in.getDirection().normalized(), actual_normal), k) * SchlickGGX(cosine, k) * SchlickFresnel(F0, cosine) * cosine;//albedo * actual_normal_and_pdf.w();

        record_ptr->cur_state = hit_record::specular_or_refract_isotropic;

        record_ptr->scatter_pdf = actual_normal_and_pdf.w();;    // 内积中的法线必须是单位向量；注意pdf要在积分域上归一化；采样漫反射用的pdf就是brdf的归一化；要作为分母故不能为0
    }
private:
    QVector3D albedo;   // 反射率即光没被吸收的概率，随表面颜色的变化而不同，要赋给每条打到该material上的光线指向的hit_record中的衰减率attenuation成员
// 其实应使用一组波长而不是RGB，因为物体呈现某种颜色是因为它们对不同波长的光的反射率即 1-吸收率 不同。可以将R，G和B视为长，中和短波长的光的混合
    double roughness;    // 属于[0, 1]的模糊度，值越大越模糊，用于微调反射方向，一般物体越大模糊度应越大；等于1 - 平滑度
    double metalness;
};
// 用立体角定义光散射的方向，它的pdf就是brdf，除此之外也会随打中的是表面还是体volume即击中点的材质而变化

class dielectric : public material {    // 水和玻璃，折射和 镜面反射与漫反射混合 随机二选一，之所以不能两个都要是因为传播一条以上的光线就会造成递归爆炸
public:
    dielectric(double ratio) : index_of_refraction(ratio) {}
    virtual void scatter(ray &r_in) const override {
        std::shared_ptr<hit_record> record_ptr = r_in.getRecord();
        record_ptr->BRDF_multiply_cos_or_emmited = QVector3D(1.0, 1.0, 1.0); // 衰减率为1即不衰减
        QVector3D normal_direction = r_in.getDirection().normalized(); // 必须有这句话，因为reflect和refract都要求传入的是单位向量
        double cos_theta = QVector3D::dotProduct(-normal_direction, record_ptr->normal);
        double sin_theta = sqrt(1.0 - pow(cos_theta, 2));   // sin(入射角)
        double sin_degree_ratio = record_ptr->outer2inner ? (1.0 / index_of_refraction) : index_of_refraction;
        record_ptr->cur_state = hit_record::specular_or_refract_isotropic;
        // 需要的是 sin(折射角)/sin(入射角) 即 入射介质的折射率/出射介质的折射率 的值；光从表面外沿与法线夹角为θ1射向表面内即outer2inner为true时，该值应是 1.0/介质的折射率index_of_refraction
        bool cannot_refract = sin_degree_ratio * sin_theta > 1.0;   // sin(θ2)/sin(θ1) * sinθ1 > 1时表明出现了全内反射，即光线不被折射只被反射，这就是有时水-空气边界会充当完美的镜子的原因
        // record_ptr->normal始终指向入射光线起始点所在方向，这样在计算reflect、refract和对光源采样时可以直接使用
        if (cannot_refract || reflectance(cos_theta, sin_degree_ratio) > random_double()){
            // 出现全内反射或 镜面反射占所有反射即镜面反射+漫反射（漫反射即没被吸收的折射与微表面的多次镜面反射之和）的比例高 时镜面反射，是否比例高的标准由random_double()随机指定；否则折射
            record_ptr->scatter_direction = reflect(normal_direction, record_ptr->normal);
        }
        else{
            record_ptr->scatter_direction = refract(normal_direction, record_ptr->normal, sin_degree_ratio);
        }
        record_ptr->scatter_pdf = 1;
    }
    double reflectance(double cosine, double refraction_ratio) const{   // 镜面反射占所有反射即 镜面反射+漫反射（漫反射即没被吸收的折射与微表面的多次反射之和）的比例
        // 使用菲涅尔Fresnel效应得到该视角即反射方向上镜面反射占所有反射即 镜面反射+漫反射（漫反射即没被吸收的折射与微表面的多次反射之和）的比例，该比例被称为反射率
        // 绝缘体如玻璃和导体如金属的菲涅尔项不同，导体主要是镜面反射，漫反射很少，这就是为什么镜子是银和铜的；抛光的金属material比镜子稍微粗糙一些，反射光集中在镜面反射附近
        // （例：站在湖边时，近处的水很清澈因为反射率低，远处的水很浑浊因为反射率高；坐车时，近处的车窗能看到外面因为反射率低，远处的车窗看不到外面因为反射率高）
        // Schlik菲涅尔近似公式：F0 + (1–F0)*(1–v·n)^5，其中F0是用于控制菲涅尔反射的强度的反射系数（<1，可以为((1 - 入射介质的折射率/出射介质的折射率) / (1 + 入射介质的折射率/出射介质的折射率))^2），
        // v是视角即反射方向，n是法线方向（v·n=入射方向·n），在反射方向和法线方向成0度时为F0，成90度即掠射角时为1
        // 传入的第二个参数是 sin(折射角)/sin(入射角) 即 入射介质的折射率/出射介质的折射率
        double F0 = pow((1 - refraction_ratio) / (1 + refraction_ratio), 2);
        return F0 + (1 - F0) * pow((1 - cosine), 5);
    }
private:
    double index_of_refraction; // 介质的折射率都是指从真空射向介质即从表面外射向表面内时，sin(入射角)/sin(折射角) 即 折射介质的折射率/入射介质即真空的折射率1
};  // 使用dielectric的一个有趣且简单的技巧是使用负半径，这时表面法线会指向内部且几何形状不会受到影响，可用来实现空心玻璃球内部的气泡

// 早期的简单光线追踪器使用抽象的光源，例如空间中的点或方向。现代方法具有更多基于物理的灯光，这些灯光具有位置和大小
// 要创建这样的光源，需要能够将任何常规物体变成可以向场景中发光的物体
class diffuse_light : public material  {    // 单向灯
    public:
        diffuse_light(std::shared_ptr<texture> a) : emit_texture(a) {}
        diffuse_light(const QVector3D &color) : emit_texture(std::make_shared<solid_color>(color)) {}

        virtual void scatter(ray &r_in) const override {
            std::shared_ptr<hit_record> record_ptr = r_in.getRecord();
            record_ptr->cur_state = hit_record::emittied;
            if(record_ptr->outer2inner){
            //像背景一样，它仅告诉射线是什么颜色，并且不进行反射
                record_ptr->BRDF_multiply_cos_or_emmited = emit_texture->value(record_ptr->u, record_ptr->v, record_ptr->intersection);
            }
            // 天花板上的灯光周围发出嘈杂的声音，是因为灯光是双面的，并且灯光和天花板之间的空间很小
            else{
                record_ptr->BRDF_multiply_cos_or_emmited = QVector3D(0, 0, 0);
            }
        };

    private:
        std::shared_ptr<texture> emit_texture;
};

class isotropic : public material { // 各向同性的散射函数选取均匀的随机方向
    public:
        isotropic(QVector3D c) : albedo(std::make_shared<solid_color>(c)) {}
        isotropic(std::shared_ptr<texture> a) : albedo(a) {}

        virtual void scatter(ray &r_in) const override {
            std::shared_ptr<hit_record> record_ptr = r_in.getRecord();
            record_ptr->scatter_direction = random_in_unit_sphere();
            // 这个材质的散射原理和漫反射磨砂材质的大同小异，均属于碰撞点转换为新视点，沿任意方向发射新的视线
            // 只不过漫反射的视线方向向量指向外相切球体表面，而isotropic的视线方向指向以碰撞点为球心的单位球体表面
            // 区别就在于漫反射的散射光线不可能指到物体内部，它一定是散射到表面外部（视线方向指向外切球体表面）
            //isotropic材质的散射光线可以沿原来的方向一往前，以此视线透光性
            //因为烟雾内部只是颗粒而不存在真正不可穿透的几何实体，所以漫反射实体不可穿透，只能散射到表面外部，而烟雾可穿透
            record_ptr->cur_state = hit_record::specular_or_refract_isotropic;
            record_ptr->BRDF_multiply_cos_or_emmited = albedo->value(record_ptr->u, record_ptr->v, record_ptr->intersection);
            record_ptr->scatter_pdf = 1;
        }

    private:
        std::shared_ptr<texture> albedo;
};

#endif // MATERIAL_H
