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

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};

layout (location = 0) in VertexData v_input;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;

uniform Material u_material;
uniform DirectionalLight u_sun;
uniform vec3 u_cameraPosition = vec3(0.f);
uniform int u_entityID = -1;

float CalculateDiffuse(vec3 N, vec3 lightDirection)
{
    const float diffuseFactor = max(dot(N, lightDirection), 0.f);
    return diffuseFactor;
}

float CalculateSpecular(vec3 N, vec3 lightDirection)
{
    const vec3 viewDirection = normalize(u_cameraPosition - v_input.position);
    const vec3 reflectDirection = reflect(-lightDirection, N);
    const float shininess = 32.f;
    const float specularFactor = pow(max(dot(viewDirection, reflectDirection), 0.f), shininess);
    return specularFactor;
}

vec4 GetColorAlbedo()
{
    const vec4 albedoTextureColor = texture(u_material.albedoTexture, v_input.texCoord);
    const vec4 albedo = u_material.albedo * albedoTextureColor;
    return albedo;
}

void main()
{
    const vec3 N = normalize(v_input.normal);
    const vec3 ambient = vec3(0.4f);
    const vec4 albedo = GetColorAlbedo();
    const vec3 lightDirection = normalize(-u_sun.direction); 

    const float diffuseFactor = CalculateDiffuse(N, lightDirection);
    const vec3 diffuse = diffuseFactor * u_sun.color;

    const float specularFactor = CalculateSpecular(N, lightDirection);
    const vec3 specular = specularFactor * u_sun.color;

    const vec3 lighting = ambient + (diffuse + specular) * u_sun.intensity;
    finalColor = vec4(lighting, 1.f) * albedo;
    entityID = u_entityID;
}
