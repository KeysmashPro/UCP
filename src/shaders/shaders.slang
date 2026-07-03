/* shaders/shaders.slang | Copyright(C) Neocortex 2026 */

struct TimeData {
    float curr;
    float prev;
    uint frame;
    uint i_frame;
};

[[vk::binding(0, 0)]]
cbuffer TimeBuffer : register(b0) {
    TimeData time_data;
}

[shader("vertex")]
void vertexMain(
    uint vertexID : SV_VertexID,
    out float4 position : SV_Position,
    out float3 color : COLOR
) {
    float2 positions[3] = {
        float2(0.0, -0.5),
        float2(0.5, 0.5),
        float2(-0.5, 0.5)
    };
    
    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    
    float scale = 1.0 + 0.3 * sin(time_data.curr * 1.0);
    
    position = float4(positions[vertexID] * scale, 0.0, 1.0);
    color = colors[vertexID];
}

[shader("fragment")]
float4 fragmentMain(
    float3 color : COLOR
) : SV_Target {
    return float4(color, 1.0);
}
