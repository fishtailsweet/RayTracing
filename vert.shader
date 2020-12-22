#version 330    // 版本为3.3
layout(location = 0) in vec3 pos;   // 使用VAO索引0上的缓冲区，每3个元素为一个名为pos的vec3
layout(location = 1) in vec2 iuv;   // 使用VAO索引1上的缓冲区，每3个元素为一个名为iuv的vec2
out vec2 ouv;   // 输出给fragment shader
void main() {
    gl_Position = vec4(pos, 1.0f);  // 第四个坐标是为了得到齐次坐标
    ouv = iuv;
}
