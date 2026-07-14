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
    float xy = (ubo.x_res / ubo.y_res);
    float x = (fragUV.x) * xy;
    float y = (fragUV.y);
    x -= ((ubo.x_pos / ubo.x_res) * 2.0 - 1.0) * xy;
    y -= (ubo.y_pos / ubo.y_res) * 2.0 - 1.0;

    float dst = (x * x) + (y * y);
    float wave = 2.0 / ((sin(dst * 4.0 - ubo.curr * 3.0) + 1.0) * 8.0);

    float r = wave * (sin(ubo.curr * 0.5) + 1.03) / 2.0;
    float g = wave * (sin(ubo.curr * 0.5 + 2.0) + 1.03) / 2.0;
    float b = wave * (cos(ubo.curr * 0.5) + 1.03) / 2.0;

    outColor = vec4(r, g, b, 1.0);
}
