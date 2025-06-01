#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in float texIndex;

out VERTEX_DATA 
{
    vec3 position;
    vec3 color;
    vec2 texCoord;
    float texIndex;
} data;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    data.position = position;
    data.color = color;
    data.texCoord = texCoord;
    data.texIndex = texIndex;

    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
}
