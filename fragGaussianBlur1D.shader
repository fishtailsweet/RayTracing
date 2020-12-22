#version 330
in vec2 ouv[5];    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
uniform float gaussian_weight[3];   // 只读，用于求梯度向量的x分量的滤波器
out vec4 color;

void main() {
    vec3 sum = texture2D(imgTexture, ouv[0]).rgb * gaussian_weight[0];
    for(int i = 1; i < 3; ++i){
        sum += texture2D(imgTexture, ouv[2 * i - 1]).rgb * gaussian_weight[i];
        sum += texture2D(imgTexture, ouv[2 * i]).rgb * gaussian_weight[i];
    }
    color = vec4(sum, 1.0);
}
