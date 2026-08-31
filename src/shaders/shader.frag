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
    float x_center = ((ubo.x_pos / ubo.x_res) * 2.0 - 1.0) * xy;
    float y_center = (ubo.y_pos / ubo.y_res) * 2.0 - 1.0;
    float time = (x_center + y_center) * 4.0;
    x -= x_center;
    y -= y_center;

    float dst = (x * y) + (y * x);
    float wave = 1.0 / ((sin(dst * 4.0 - time * 3.0) + 1.0) * 8.0);

    float r = wave * (sin(time * 0.5) + 1.03) / 2.0;
    float g = wave * (sin(time * 0.5 + 2.0) + 1.03) / 2.0;
    float b = wave * (cos(time * 0.5) + 1.03) / 2.0;

    outColor = vec4(r, g, b, 1.0);
}
