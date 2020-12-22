#version 330
in vec2 ouv;    // 只读，必须与vertex shader中输出的vec2变量同名
uniform sampler2D imgTexture;   // uniform表明整个渲染过程中值相同，*只读
uniform mat2x2 trans;   // 只读
uniform vec2 tex_size;  // 只读
out vec4 color;
void main() {
    vec2 rotate_uv = trans * ((ouv - vec2(0.5, 0.5)) * tex_size);   // 必须以图像中心为中心进行旋转
    vec4 color_up = (int(rotate_uv.x) + 1 - rotate_uv.x) * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5)) +
            (rotate_uv.x - int(rotate_uv.x)) * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y)) / tex_size + vec2(0.5, 0.5));
    vec4 color_down = (int(rotate_uv.x) + 1 - rotate_uv.x) * texture2D(imgTexture, vec2(int(rotate_uv.x), int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5)) +
            (rotate_uv.x - int(rotate_uv.x)) * texture2D(imgTexture, vec2(int(rotate_uv.x) + 1, int(rotate_uv.y) + 1) / tex_size + vec2(0.5, 0.5));
    color = (int(rotate_uv.y) + 1 - rotate_uv.y) * color_up + (rotate_uv.y - int(rotate_uv.y)) * color_down;
}
