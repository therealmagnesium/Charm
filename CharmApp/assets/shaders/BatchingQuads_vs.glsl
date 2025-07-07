#version 450 core

struct VertexData
{
    vec3 position;
    vec3 color;
    vec2 texCoord;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in uint texIndex;
layout (location = 4) in int entityID;

layout (location = 0) out VertexData data;
layout (location = 3) out flat uint v_texIndex;
layout (location = 4) out flat int v_entityID;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    data.position = position;
    data.color = color;
    data.texCoord = texCoord;
    v_texIndex = texIndex;
    v_entityID = entityID;

    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
}
