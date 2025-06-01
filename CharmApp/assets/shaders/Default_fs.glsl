#version 450 core
in VERTEX_DATA
{
    vec3 position;
} data;

out vec4 finalColor;

void main()
{
    vec3 color = vec3(0.8f, 0.5f, 0.25f);
    vec3 result = pow(color, vec3(1.f / 2.2f));
    finalColor = vec4(result, 1.f);
}
