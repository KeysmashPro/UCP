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
    float xc = fragUV.x * xy;
    float yc = fragUV.y;
    float x_center = ((ubo.x_pos / ubo.x_res) * 2.0 - 1.0) * xy;
    float y_center = (ubo.y_pos / ubo.y_res) * 2.0 - 1.0;
    float time = ubo.curr * 1;

    float x = xc + sin(yc * 4 + time) * 0.17;
    float y = yc + sin(xc * 4 + time) * 0.17;
    float w = mod(floor(x * 4) + floor(y * 4), 2.0);
    w += (sin(yc * 4 + time) + cos(xc * 4 + time)) * 0.1;

    outColor = vec4(w, w, w, 1.0);
}
