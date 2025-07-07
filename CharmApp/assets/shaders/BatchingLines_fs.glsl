#version 450 core

struct VertexData
{
    vec3 position;
    vec3 color;
};

layout (location = 0) in VertexData data;
layout (location = 0) out vec4 finalColor;

void main()
{
    vec3 result = pow(data.color, vec3(1.f / 2.2f));
    finalColor = vec4(result, 1.f);
}

