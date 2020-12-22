#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明整个渲染过程中值相同，*只读
uniform float rotate_level;   // 只读
uniform vec2 center;
out vec4 color;

void main() {
    vec2 rotate_vector = ouv - center;      // 最远是(0.5, 0.5)，长度为sqrt(2)/2

    float degree_angle = rotate_level / (length(rotate_vector) + 0.001);  // 也可以是mix(rotate_level, 0, length(rotate_vector) * 1.4)，mix(a, b, c) = (1-c)a + cb
    // +0.001防止除0错误
    float radian_angle = radians(-degree_angle);
    float cos_value = cos(radian_angle);
    float sin_value = sin(radian_angle);
    // <QtMath>自带M_PI和角度转弧度float qDegreesToRadians(float degrees)；内置三角函数用的都是弧度
    vec2 rotate_uv = vec2(cos_value * rotate_vector.x - sin_value * rotate_vector.y, sin_value * rotate_vector.x + cos_value * rotate_vector.y);    // 旋转
    color = texture2D(imgTexture, rotate_uv + center);
}
// 为了让偏移变得随机，我们就要引入一个能够随机化输出的东东，也就是噪声图
// 只需要用一个连续变化的值去采噪声图，就可以得到不连续的随机输出偏移值，而时间恰好就是一个连续变化的值，故时间变量常用来采样噪声图
