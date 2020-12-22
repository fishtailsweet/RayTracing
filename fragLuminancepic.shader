#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
out vec4 color;
void main() {
    vec4 tex_color = texture2D(imgTexture, ouv);
    float luminance = 0.2125 * tex_color.r + 0.7154 * tex_color.g + 0.0721 * tex_color.b;
    color = vec4(luminance, luminance, luminance, tex_color.w);
}
