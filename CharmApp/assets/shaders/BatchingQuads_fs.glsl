#version 450 core

struct VertexData
{
    vec3 position;
    vec3 color;
    vec2 texCoord;
};

layout (location = 0) in VertexData data;
layout (location = 3) in flat uint v_texIndex;
layout (location = 4) in flat int v_entityID;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;

uniform sampler2D textures[32];

void main()
{
    vec3 result = texture(textures[v_texIndex], data.texCoord).xyz * data.color;
    result = pow(result, vec3(1.f / 2.2f));

    finalColor = vec4(result, 1.f);
    entityID = v_entityID;
}
