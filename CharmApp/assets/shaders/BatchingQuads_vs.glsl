#version 450 core

struct VertexData
{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec2 tilingFactor;
};

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texCoord;
layout (location = 3) in vec2 a_tilingFactor;
layout (location = 4) in uint a_texIndex;
layout (location = 5) in int a_entityID;

layout (location = 0) out VertexData v_output;
layout (location = 4) out flat uint v_texIndex;
layout (location = 5) out flat int v_entityID;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    v_output.position = a_position;
    v_output.color = a_color;
    v_output.texCoord = a_texCoord;
    v_output.tilingFactor = a_tilingFactor;
    v_texIndex = a_texIndex;
    v_entityID = a_entityID;

    gl_Position = projectionMatrix * viewMatrix * vec4(a_position, 1.f);
}
