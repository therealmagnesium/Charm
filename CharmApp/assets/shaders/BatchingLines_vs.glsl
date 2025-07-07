#version 450 core

struct VertexData
{
    vec3 position;
    vec3 color;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 0) out VertexData v_data;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    v_data.position = position;
    v_data.color = color;

    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
}

