#version 450 core

struct VertexData
{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec2 tilingFactor;
};

layout (location = 0) in VertexData v_input;
layout (location = 4) in flat uint v_texIndex;
layout (location = 5) in flat int v_entityID;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;

uniform sampler2D textures[32];

void main()
{
    vec4 result = texture(textures[v_texIndex], v_input.texCoord * v_input.tilingFactor) * v_input.color;
    //result = pow(result, vec4(1.f / 2.2f));

    finalColor = result;
    entityID = v_entityID;
}
