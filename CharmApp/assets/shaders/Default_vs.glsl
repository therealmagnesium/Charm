#version 450 core
layout (location = 0) in vec3 position;

out VERTEX_DATA 
{
    vec3 position;
} data;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    data.position = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
}
