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
    vec2 p = (fragUV - 0.5) * 1.5;

    p.x *= ubo.x_res / ubo.y_res;

    float x = (p.x + 1.2 * sin(ubo.curr) + 1.0);
    float y = (p.y + 0.7 * cos(ubo.curr) + 0.4);
    float d2 = x* x + y * y;
    float wave =  2 / ((sin(d2 * 2.0 - ubo.curr * 3) + 1.0) * 8);
    float r = wave * (sin(ubo.curr * 0.5) + 1.03) / 2;
    float g = wave * (sin(ubo.curr * 0.5 + 2) + 1.03) / 2;
    float b = wave * (cos(ubo.curr * 0.5) + 1.03) / 2;

    outColor = vec4(r, g, b, 1.0);
}
