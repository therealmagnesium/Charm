#version 450 core

layout (location = 0) in vec3 worldPosition;
layout (location = 1) in vec3 localPosition;
layout (location = 2) in vec3 color;
layout (location = 3) in float thickness;
layout (location = 4) in float fade;

out VERTEX_DATA
{
    vec3 localPosition;
    vec3 color;
    float thickness;
    float fade;
} data;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    data.localPosition = localPosition;
    data.color = color;
    data.thickness = thickness;
    data.fade = fade;

    gl_Position = projectionMatrix * viewMatrix * vec4(worldPosition, 1.f);
}
