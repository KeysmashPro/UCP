#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO {
    float curr;
    float prev;
    uint  frame;
    uint  image;
    float x_res;
    float y_res;
    float x_pos;
    float y_pos;
    float scroll;
    uint  buttons;
} ubo;

void main() {
    vec2 p = (fragUV - 0.5) * 2.0;

    p.x *= ubo.x_res / ubo.y_res;

    float d2 = p.x * p.x + p.y * p.y;
    float wave = sin(d2 * 2.0 - ubo.curr * 3.0);
    float r = wave * cos(ubo.curr * 0.4);
    float g = wave * sin(ubo.curr * 0.2);
    float b = wave * sin(ubo.curr * 0.8);

    outColor = vec4(r, g, b, 1.0);
}
