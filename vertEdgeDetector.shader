#version 330    // 版本为3.3
layout(location = 0) in vec3 pos;   // 使用VAO索引0上的缓冲区，每3个元素为一个名为pos的vec3
layout(location = 1) in vec2 iuv;   // 使用VAO索引1上的缓冲区，每3个元素为一个名为iuv的vec2
out vec2 ouv[9];   // 输出给fragment shader，可以声明数组
uniform vec2 tex_size;  // 只读，(1/width, 1/height)，需要通过它在vertex shader中得到屏幕图像中与该顶点对应的归一化纹理坐标相邻的8个归一化纹理坐标
void main() {
    ouv[0] = iuv + 1 / tex_size * vec2(1, -1);	// 得到屏幕图像中与该顶点对应的归一化纹理坐标相邻的8个归一化纹理坐标，是逐顶点边缘检测（足够了）；右下
    ouv[1] = iuv + 1 / tex_size * vec2(0, -1);	// 下
    ouv[2] = iuv + 1 / tex_size * vec2(-1, -1);    // 左下
    ouv[3] = iuv + 1 / tex_size * vec2(1, 0);	// 右
    ouv[4] = iuv;
    ouv[5] = iuv + 1 / tex_size * vec2(-1, 0);	// 左
    ouv[6] = iuv + 1 / tex_size * vec2(1, 1);	// 右上
    ouv[7] = iuv + 1 / tex_size * vec2(0, 1);	// 上
    ouv[8] = iuv + 1 / tex_size * vec2(-1, 1);	// 左上
    gl_Position = vec4(pos, 1.0f);  // 第四个坐标是为了得到齐次坐标
}
/*亮度luminance = 0.2125 * Red + 0.7154 * Green + 0.0721 * Blue，然后将RGB通道都设为亮度值，透明度a通道设为对屏幕图像采样得到的透明度值，则可得到灰度效果；亮度图像是非常常用的，如边缘检测等
梯度非线性滤波器是通过对每个位置第一步卷积地求梯度向量（线性），第二步非线性地对第一步得到的梯度向量求范数或两分量的绝对值和并赋给新图像的该位置实现的，一般作为自动检测的预处理，可达到增强缺陷和小突变并去除灰度值渐变的块的目的
梯度向量 grad 是 [对 x 的偏导 对 y 的偏导]'，它指出了在该位置处灰度值最大变化率的方向，不是各向同性的，可用于确定边缘的强度和方向；边缘的强度为梯度向量的范数或更适合计算的两分量的绝对值和，用于边缘检测时要给定强度阈值，然后计算每个位置上滤波后的值，< 阈值则置0，>= 阈值则置1；边缘的方向垂直于该位置梯度向量的方向，可通过 arctan(对 y 的偏导 / 对 x 的偏导) 求出边缘角度
在梯度向量方向上变换率的值为梯度向量的范数，是各向同性的，用 |对 x 的偏导| + |对 y 的偏导| 代替范数更适合计算
一般使用 Sobel 线性算子求梯度向量的两分量，即令对 x 的偏导为 (f(x + 1, y - 1) + f(x + 1, y) + f(x + 1, y + 1)) - (f(x - 1, y - 1) + f(x - 1, y) + f(x - 1, y + 1))，滤波器形式为 [-1, -2, -1; 0, 0, 0; 1, 2, 1]；对 y 的偏导为 (f(x - 1, y + 1) + f(x, y + 1) + f(x + 1, y + 1)) - (f(x - 1, y - 1) + f(x, y - 1) + f(x + 1, y - 1))，滤波器形式为 [-1, 0, 1; -2, 0, 2;-1, 0, 1]
Sobel 线性算子的中心系数为2的原因是突出中心点可以达到平滑的目的（常用），系数之和一定为0因为它属于微分算子，但不是各向同性的
Sobel算子边缘检测的后处理shader，生成一个对边缘加深过的背景为原屏幕图像的像素和一个只显示边缘的背景为纯色的像素，可以指定二者的混合比例以控制边缘叠加在原屏幕图像上的程度，注意逐顶点边缘检测就足够了，即在vertex shader中计算与该顶点对应的纹理坐标相邻的纹理坐标：*/
