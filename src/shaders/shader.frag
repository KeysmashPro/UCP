#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform TimeBuffer {
    float curr;
    float prev;
    uint  frame;
    uint  image;
    float width;
    float height;
    float x_pos;
    float y_pos;
    float scroll;
    uint  buttons;
} time_data;

void main() {
    outColor = vec4(0, 0, 0, 0);
}
