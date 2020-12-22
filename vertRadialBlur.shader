#version 330    // 版本为3.3，必须在第一行，在这之前不能有注释
layout(location = 0) in vec3 pos;   // 使用VAO索引0上的缓冲区，每3个元素为一个名为pos的vec3
layout(location = 1) in vec2 iuv;   // 使用VAO索引1上的缓冲区，每3个元素为一个名为iuv的vec2
out vec2 ouv[5];   // 输出给fragment shader，可以声明数组
uniform vec2 tex_size;  // 只读，(1/width, 1/height)，需要通过它在vertex shader中得到屏幕图像中与该顶点对应的归一化纹理坐标相邻的8个归一化纹理坐标
uniform float blur_size;
uniform vec2 hor_or_ver;    // 用于表示这次是水平模糊还是竖直模糊
void main() {
    ouv[0] = iuv;
    ouv[1] = iuv + 1 / tex_size * hor_or_ver * blur_size;	// // 通过控制采样距离控制模糊程度，采样距离越大越模糊
    ouv[2] = iuv - 1 / tex_size * hor_or_ver * blur_size;    // 左下
    ouv[3] = iuv + 1 / tex_size * 2 * hor_or_ver * blur_size;	// 右
    ouv[4] = iuv - 1 / tex_size * 2 * hor_or_ver * blur_size;
    gl_Position = vec4(pos, 1.0f);  // 第四个坐标是为了得到齐次坐标
}
/*有理想滤波器（最尖锐），布特沃斯滤波器（阶数越高越尖锐）和高斯滤波器（最平滑），低通用于平滑，定义为 1 - 低通 的高通用于锐化，因为是可分且对称的故在使用时均可将二维卷积化为两次用一维滤波器的一维卷积，且用它进行相关和卷积得到的结果是相同的；注意上述空间域滤波器的系数之和必须为1，即要整体除以系数之和以归一化；滤波器越大，效果越明显
低通滤波器截止频率越小模糊程度越大，对应的空间域低通滤波器模板也越大，常作为预处理的一部分；高通滤波器截止频率越小锐化程度越大
高斯低通GLPF为e^(-距中心距离的平方 / (2 * 标准差的平方))，标准差σ就是截止频率，没有振铃，效果好因为邻域内距当前像素越近的像素影响越大
RenderTexture有静态成员函数GetTemporary和ReleaseTemporary以创建和释放RT；RT有filterMode成员变量以指定采样策略如双线性插值FilterMode.Bilinear
σ = 1的高斯模糊的后处理Shader，因为是可分且对称的故可将二维卷积化为两次用一维滤波器的一维卷积，先对屏幕图像用在竖直方向上卷积的第一个Pass，将结果输出到中间的临时RT（分辨率可以比屏幕小即降采样以实现二次模糊，这样在第一个Pass中就会按新的分辨率进行采样，需要处理的像素少了很多），然后对中间RT用在水平方向上卷积的第二个Pass，可通过调整高斯滤波的次数来控制模糊程度；且因为是对称的故无需将滤波器整个存储而只需存其中几个数；注意逐顶点高斯模糊就足够了，即在vertex shader中计算与该顶点对应的纹理坐标相邻的纹理坐标*/
// Uniform Buffer Object和普通传递uniform变量方式的优势在于不同着色器可以分享数据、因为buffer是GPU的内存空间、着色器都是可以访问的。而不同的pass本质也就是不同的shader，所以都能访问UBO
