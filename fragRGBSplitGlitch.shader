#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明对于每个片元，值都相同，*只读
uniform float split_level;
uniform vec2 split_dir;
out vec4 color;
void main() {
    vec4 tex_color = texture2D(imgTexture, ouv);
    color = vec4(texture2D(imgTexture, ouv + split_level * split_dir).r, tex_color.g, texture2D(imgTexture, ouv + split_level * split_dir).b, tex_color.w);
}
