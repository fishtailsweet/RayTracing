#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
uniform int blur_num;   // 采样次数
uniform float blur_size;    // 每次跳跃的幅度
uniform vec2 center;
out vec4 color;

void main() {
    vec2 dir = ouv - center;
    vec3 average;
    for(int i = 1; i <= blur_num; ++i){
        vec2 temp_uv = ouv + blur_size * dir * i;
        average += texture2D(imgTexture, temp_uv).rgb;
    }
    color = vec4(average / blur_num, 1.0);    // 进行一定次数的迭代采样，最终将采样得到的RGB值累加，并除以迭代次数
}
// 采样距离越靠外，距离越远；越靠里，距离越近；这样边缘地方的采样距离大，模糊；中心点采样距离小，不模糊
// 采样点越靠近中心越密集，越远离中心越稀疏，最后，该像素点的输出就是这些采样点的均值。这样，在靠近中心点的位置，采样距离小，几乎为0，也就不会模糊
// 而越靠近边界的位置，采样的距离越大，图像也就会越模糊
// 这是径向模糊-缩放（即方向模糊），同理可以有径向模糊-旋转，即采样的是一些旋转点
// 移动设备上，GPU不强，但是分辨率极高
