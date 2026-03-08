#version 450 core

struct VertexData
{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec3 normal;
};

layout (location = 0) in VertexData v_input;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;
layout (location = 4) in flat int v_entityID;

void main()
{
    const vec3 outlineColor = vec3(1.f, 0.6f, 0.4f);
    finalColor = vec4(outlineColor, 1.f);
    entityID = v_entityID;
}

