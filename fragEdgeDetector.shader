#version 330
in vec2 ouv[9];    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
uniform mat3x3 filter_x;   // 只读，用于求梯度向量的x分量的滤波器
uniform mat3x3 filter_y;   // 只读，用于求梯度向量的y分量的滤波器
uniform float edge_level;   // 只读，用于控制边缘叠加在原屏幕图像上的程度，值为0时程度最低，值为1时程度最高只显示边缘
uniform vec4 edge_color;    // 边缘颜色
uniform vec4 background_color;  // 指定只显示边缘的像素的背景颜色
out vec4 color;
float luminance(vec4 color){	// rgb转亮度
    return 0.2125 * color.r + 0.7154 * color.g + 0.0721 * color.b;
}

float Sobel(){	// 用两个滤波器分别求梯度向量的x、y分量
    float edge_x = 0;
    float edge_y = 0;
    float tex_luminance;
    for(int i = 0; i < 9; ++i){
        tex_luminance = luminance(texture2D(imgTexture, ouv[i]));	// 得到亮度图像，在亮度图像上边缘检测
        edge_x += tex_luminance * filter_x[i / 3][i % 3];	// 用求梯度向量的x分量的滤波器进行卷积
        edge_y += tex_luminance * filter_y[i / 3][i % 3];	// 用求梯度向量的y分量的滤波器进行卷积
    }
    return abs(edge_x) + abs(edge_y);   // |对 x 的偏导| + |对 y 的偏导|，值越大代表该像素越是边缘
}

void main() {
    float edge = Sobel();	// |对 x 的偏导| + |对 y 的偏导|，值越大代表该像素越是边缘
    vec4 with_edge_color = mix(texture2D(imgTexture, ouv[4]), edge_color, edge);    // 生成一个对边缘加深过的背景为原屏幕图像的像素；mix(a, b, c) = (1-c)a + cb
    vec4 only_edge_color = mix(background_color, edge_color, edge);	// 生成一个只显示边缘的背景为纯色的像素
    color = mix(with_edge_color, only_edge_color, edge_level);	// 将二者混合，通过edge_level控制边缘叠加在原屏幕图像上的程度，值为0时程度最低，值为1时程度最高只显示边缘
}
// 上面实现的边缘检测仅利用了屏幕图像的颜色信息，而实际上纹理、阴影等信息均会影响边缘检测的效果，为了更加准确地边缘检测，往往不选择亮度图像而选择在深度纹理和法线纹理上边缘检测
