#version 450 core
in VERTEX_DATA
{
    vec3 position;
    vec3 color;
    vec2 texCoord;
    float texIndex;
} data;

out vec4 finalColor;

uniform sampler2D textures[32];

void main()
{
    vec3 result = texture(textures[int(data.texIndex)], data.texCoord).xyz * data.color;

    result = pow(result, vec3(1.f / 2.2f));
    finalColor = vec4(result, 1.f);
}
