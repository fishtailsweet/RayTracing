#ifndef PERLIN_H
#define PERLIN_H
#include "random_generator.h"
// Perlin返回类似于模糊的白噪声的内容
// Perlin噪声的一个关键部分是可重复性：它以3D点作为输入，并且始终返回相同的随机数。附近的点返回相似的数字
// Perlin噪声的另一个重要部分是它既简单又快速，因此通常作为hack来完成
// 可以用3D随机数数组平铺所有空间，然后在块中使用它们。在重复清晰的地方，您会得到一些块状的东西
// 让我们仅使用某种哈希来对此进行加扰，而不是进行平铺。这需要一些支持代码才能实现：
class perlin {
    public:
        perlin() {
            // 这看起来还是有些块状，可能是因为模式的最小值和最大值始终精确落在整数x / y / z上。一个非常聪明的技巧是将随机单位矢量（而不是浮点数）放在晶格点上，并使用点积将最小和最大值移出晶格（移出整数值）
            // 因此，首先我们需要将随机浮点数更改为随机向量。这些向量是不规则方向的任何合理集合，我不会费心使它们完全一致：
            /*ranfloat = new double[point_count];
            for (int i = 0; i < point_count; ++i) {
                ranfloat[i] = random_double();
            }*/
            ranvec = new QVector3D[point_count];
            for (int i = 0; i < point_count; ++i) {
                ranvec[i] = random_vec3(-1, 1).normalized();
            }

            perm_x = perlin_generate_perm();
            perm_y = perlin_generate_perm();
            perm_z = perlin_generate_perm();
        }

        ~perlin() {
            //delete[] ranfloat;
            delete[] ranvec;
            delete[] perm_x;
            delete[] perm_y;
            delete[] perm_z;
        }
        // 为了使其平滑，我们可以线性插值：
        double noise(const QVector3D& p) const {
            /*auto i = static_cast<int>(4 * p.x()) & 255;
            auto j = static_cast<int>(4 * p.y()) & 255;
            auto k = static_cast<int>(4 * p.z()) & 255;

            return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];*/
            double u = p.x() - floor(p.x());
            double v = p.y() - floor(p.y());
            double w = p.z() - floor(p.z());
// 平滑可改善结果，但是其中具有明显的网格特征。其中一些是马赫带，这是一种已知的线性颜色插值的感知伪像
// 一个标准的技巧是使用Hermite三次来对插值进行四舍五入：
// 这样可以使图像看起来更平滑：
// u = u*u*(3-2*u);
// v = v*v*(3-2*v);
// w = w*w*(3-2*w);

            auto i = static_cast<int>(floor(p.x()));
            auto j = static_cast<int>(floor(p.y()));
            auto k = static_cast<int>(floor(p.z()));
            QVector3D c[2][2][2];

            for (int di=0; di < 2; di++)
                for (int dj=0; dj < 2; dj++)
                    for (int dk=0; dk < 2; dk++)
                        /*c[di][dj][dk] = ranfloat[
                            perm_x[(i+di) & 255] ^
                            perm_y[(j+dj) & 255] ^
                            perm_z[(k+dk) & 255]
                        ];*/
                        c[di][dj][dk] = ranvec[
                            perm_x[(i+di) & 255] ^
                            perm_y[(j+dj) & 255] ^
                            perm_z[(k+dk) & 255]
                        ];

            return perlin_interp(c, u, v, w);
        }
// 使用具有多个总频率的复合噪声。这通常称为湍流，是反复发出噪音的总和：
// 直接使用湍流会产生一种伪装网状外观：
        double turb(const QVector3D& p, int depth=7) const {
            auto accum = 0.0;
            auto temp_p = p;
            auto weight = 1.0;

            for (int i = 0; i < depth; i++) {
                accum += weight*noise(temp_p);
                weight *= 0.5;
                temp_p *= 2;
            }
// Perlin插值的输出可以返回负值。这些负值将传递给gamma校正中的sqrt()，并变成NaNs。故需要把perlin输出转换回0到1之间。
            return fabs(accum);
        }
    private:
        static const int point_count = 256;
        //double* ranfloat;
        QVector3D *ranvec;
        int* perm_x;
        int* perm_y;
        int* perm_z;

        static int* perlin_generate_perm() {
            auto p = new int[point_count];

            for (int i = 0; i < perlin::point_count; ++i)
                p[i] = i;

            permute(p, point_count);

            return p;
        }

        static void permute(int* p, int n) {
            for (int i = n - 1; i > 0; --i) {
                int target = random_int(0, i);
                int tmp = p[i];
                p[i] = p[target];
                p[target] = tmp;
            }
        }
        /*static double trilinear_interp(double c[2][2][2], double u, double v, double w) {   // 三维中用三线性插值
            auto accum = 0.0;
            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++)
                        accum += (i*u + (1-i)*(1-u))*
                            (j*v + (1-j)*(1-v))*
                            (k*w + (1-k)*(1-w))*c[i][j][k];
            return accum;
        }*/
        // 插值变得更加复杂：
        static double perlin_interp(QVector3D c[2][2][2], double u, double v, double w) {
            auto uu = u*u*(3-2*u);
            auto vv = v*v*(3-2*v);
            auto ww = w*w*(3-2*w);
            auto accum = 0.0;

            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++) {
                        QVector3D weight_v(u-i, v-j, w-k);
                        accum += (i*uu + (1-i)*(1-uu))
                            * (j*vv + (1-j)*(1-vv))
                            * (k*ww + (1-k)*(1-ww))
                            * QVector3D::dotProduct(c[i][j][k], weight_v);
                        }

            return accum;
       }
};

#endif // PERLIN_H
