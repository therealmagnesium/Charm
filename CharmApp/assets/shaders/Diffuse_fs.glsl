#version 450 core

struct VertexData
{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec3 normal;
};

struct Material
{
    vec4 albedo;
    sampler2D albedoTexture;
};

layout (location = 0) in VertexData v_input;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;

uniform Material u_material;
uniform int u_entityID = -1;

void main()
{
    vec4 albedo = texture(u_material.albedoTexture, v_input.texCoord);
    finalColor = length(albedo) > 1 ? u_material.albedo * albedo : u_material.albedo;
    entityID = u_entityID;
}
