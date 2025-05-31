/*
#version 450 core
layout (location = 0) in vec3 position;

out VERTEX_DATA 
{
    vec3 position;
} data;

void main()
{
    data.position = position;
    gl_Position = vec4(position, 1.f);
}*/

#version 450 core
layout (location = 0) in vec3 position;

void main()
{
    gl_Position = vec4(position, 1.f);
}
