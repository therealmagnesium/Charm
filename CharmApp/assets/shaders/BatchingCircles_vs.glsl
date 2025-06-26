#version 450 core

struct VertexData
{
    vec3 localPosition;
    vec3 color;
    float thickness;
    float fade;
};

layout (location = 0) in vec3 worldPosition;
layout (location = 1) in vec3 localPosition;
layout (location = 2) in vec3 color;
layout (location = 3) in float thickness;
layout (location = 4) in float fade;
layout (location = 5) in int entityID;

layout (location = 0) out VertexData data;
layout (location = 4) out flat int v_entityID;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    data.localPosition = localPosition;
    data.color = color;
    data.thickness = thickness;
    data.fade = fade;
    v_entityID = entityID;

    gl_Position = projectionMatrix * viewMatrix * vec4(worldPosition, 1.f);
}
