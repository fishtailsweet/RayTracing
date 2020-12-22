#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
uniform mat2x2 trans;   // 只读
uniform vec2 tex_size;  // 只读
out vec4 color;
float BiCubic(float x){   // 必须在main之前有该函数的声明或定义；BiCubic的图像是关于y轴对称的，故该函数中的x都必须套一个abs
    float a = -0.5;    // a取别的值如-1也可以，但-0.5是最好的
    if(abs(x) <= 1){
        return ((a + 2) * pow(abs(x), 3) - (a + 3) * pow(abs(x), 2) + 1);
    }
    if(abs(x) > 1 && abs(x) < 2){
        return (a * pow(abs(x), 3) - 5 * a * pow(abs(x), 2) + 8 * a * abs(x) - 4 * a);
    }
    else{
        return 0;
    }
}

void main() {
    vec2 rotate_uv = trans * ((ouv - vec2(0.5, 0.5)) * tex_size);   // 必须以图像中心为中心进行旋转
    float weight_leftleft = BiCubic(rotate_uv.x - (int(rotate_uv.x) - 1));
    float weight_left = BiCubic(rotate_uv.x - int(rotate_uv.x));
    float weight_right = BiCubic(int(rotate_uv.x) + 1 - rotate_uv.x);
    float weight_rightright = BiCubic(int(rotate_uv.x) + 2 - rotate_uv.x);
    vec4 color_upup = weight_leftleft * texture2D(imgTexture, vec2(int(rotate_uv.x) - 1, int(rotate_uv.y) - 1) / tex_size + vec2(0.5, 0.5)) + weight_left * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y) - 1) / tex_size + vec2(0.5, 0.5)) + weight_right * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y) - 1) / tex_size + vec2(0.5, 0.5)) + weight_rightright * texture2D(imgTexture, vec2(int(rotate_uv.x) + 2, int(rotate_uv.y) - 1) / tex_size + vec2(0.5, 0.5));
    vec4 color_up = weight_leftleft * texture2D(imgTexture, vec2(int(rotate_uv.x) - 1, int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5)) + weight_left * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5)) + weight_right * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5)) + weight_rightright * texture2D(imgTexture, vec2(int(rotate_uv.x) + 2, int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5));
    vec4 color_down = weight_leftleft * texture2D(imgTexture, vec2(int(rotate_uv.x) - 1, int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5)) + weight_left * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5)) + weight_right * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5)) + weight_rightright * texture2D(imgTexture, vec2(int(rotate_uv.x) + 2, int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5));
    vec4 color_downdown = weight_leftleft * texture2D(imgTexture, vec2(int(rotate_uv.x) - 1, int(rotate_uv.y) + 2) / tex_size + vec2(0.5, 0.5)) + weight_left * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y) + 2) / tex_size + vec2(0.5, 0.5)) + weight_right * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y) + 2) / tex_size + vec2(0.5, 0.5)) + weight_rightright * texture2D(imgTexture, vec2(int(rotate_uv.x) + 2, int(rotate_uv.y) + 2) / tex_size + vec2(0.5, 0.5));
    color = BiCubic(rotate_uv.y - (int(rotate_uv.y) - 1)) * color_upup + BiCubic(rotate_uv.y - int(rotate_uv.y)) * color_up + BiCubic(int(rotate_uv.y) + 1 - rotate_uv.y) * color_down + BiCubic(int(rotate_uv.y) + 2 - rotate_uv.y) * color_downdown;
}
