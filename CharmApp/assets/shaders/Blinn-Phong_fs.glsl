#version 450 core

// ---------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------
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
layout (location = 4) in flat int v_entityID;

// ---------------------------------------------------------------------------
// Outputs
// ---------------------------------------------------------------------------
layout (location = 0) out vec4 finalColor;
layout (location = 1) out int  entityID;
layout (location = 2) out vec4 bloomColor;

// ---------------------------------------------------------------------------
// Uniforms — scene
// ---------------------------------------------------------------------------
uniform Material         u_material;
uniform DirectionalLight u_sun;
uniform vec3             u_cameraPosition = vec3(0.0);
uniform float            u_ambience       = 0.0;
uniform float            u_bloomThreshold = 0.5;

// ---------------------------------------------------------------------------
// Uniforms — shadow
// ---------------------------------------------------------------------------
// sampler2DArrayShadow enables hardware PCF via texture comparison mode.
uniform sampler2DArrayShadow u_shadowMap;
uniform mat4  u_lightSpaceMatrices[4];   // one per cascade
uniform float u_shadowCascadeSplits[4];  // view-space far depths: splitDepths[i] = -cascadeFar_i
uniform mat4  u_matrixView;              // already set by BeginScene3D on blinnPhong

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
const float k_gamma        = 2.2;
const int   k_CascadeCount = 4;

// ---------------------------------------------------------------------------
// Lighting helpers
// ---------------------------------------------------------------------------
float CalculateDiffuse(vec3 N, vec3 lightDir)
{
    return max(dot(N, lightDir), 0.0);
}

float CalculateSpecular(vec3 N, vec3 lightDir)
{
    const vec3  viewDir    = normalize(u_cameraPosition - v_input.position);
    const vec3  halfwayDir = normalize(lightDir + viewDir);
    const float shininess  = 32.0;
    return pow(max(dot(N, halfwayDir), 0.0), shininess);
}

vec4 GetColorAlbedo()
{
    const vec4 texColor = pow(texture(u_material.albedoTexture, v_input.texCoord),
                              vec4(vec3(k_gamma), 1.0));
    return u_material.albedo * texColor;
}

// ---------------------------------------------------------------------------
// Shadow — Cascaded PCF
// ---------------------------------------------------------------------------
float SampleCascadePCF(vec3 projCoords, int cascade)
{
    const vec2  texelSize = vec2(1.0) / vec2(textureSize(u_shadowMap, 0).xy);
    const vec3  N         = normalize(v_input.normal);
    const vec3  L         = normalize(-u_sun.direction);
    const float cosTheta  = clamp(dot(N, L), 0.0, 1.0);
    const float bias      = max(0.005 * (1.0 - cosTheta), 0.0005);

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    {
        const vec2 offset = vec2(float(x), float(y)) * texelSize;
        // sampler2DArrayShadow: coords = (s, t, layer, compare_ref)
        shadow += texture(u_shadowMap,
                          vec4(projCoords.xy + offset,
                               float(cascade),
                               projCoords.z - bias));
    }
    return shadow / 9.0;
}

int GetCascadeIndex(vec3 worldPos)
{
    const float viewZ = (u_matrixView * vec4(worldPos, 1.0)).z;
    for (int i = 0; i < k_CascadeCount; ++i)
    {
        if (viewZ > u_shadowCascadeSplits[i])
            return i;
    }
    return k_CascadeCount - 1;
}

float CalculateShadow(vec3 worldPos)
{
    const int cascade = GetCascadeIndex(worldPos);
    const vec4 fragLightSpace = u_lightSpaceMatrices[cascade] * vec4(worldPos, 1.0);
    vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 1.0;
    return SampleCascadePCF(projCoords, cascade);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
void main()
{
    const vec3  N         = normalize(v_input.normal);
    const vec3  ambient   = vec3(u_ambience);
    const vec4  albedo    = GetColorAlbedo();
    const vec3  lightDir  = normalize(-u_sun.direction);

    const float diffuseFactor  = CalculateDiffuse(N, lightDir);
    const vec3  diffuse        = diffuseFactor * u_sun.color;

    const float specularFactor = CalculateSpecular(N, lightDir);
    const vec3  specular       = specularFactor * u_sun.color;

    const float shadowFactor   = CalculateShadow(v_input.position);
    const vec3  directLighting = (diffuse + specular) * u_sun.intensity * shadowFactor;
    const vec3  lighting       = ambient + directLighting;

    finalColor = vec4(lighting, 1.f) * albedo;
    entityID   = v_entityID;

    // ------------------------------------------------------------------
    // Bloom bright-pass — measure DIRECT contribution only.
    //
    // Bug being fixed: measuring dot(finalColor.rgb, ...) includes the
    // ambient term, so surfaces with bright albedo (Sponza white walls)
    // exceed the threshold everywhere, even where no light is hitting.
    // Direct light is the only physically valid source of HDR bloom.
    // ------------------------------------------------------------------
    const vec3  directContribution = directLighting * albedo.rgb;
    const float brightness         = dot(directContribution, vec3(0.2126, 0.7152, 0.0722));

    bloomColor = (brightness > u_bloomThreshold)
        ? vec4(finalColor.rgb, 1.0)
        : vec4(0.0, 0.0, 0.0, 1.0);
}

/*
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
layout (location = 4) in flat int v_entityID;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;
layout (location = 2) out vec4 bloomColor;

uniform Material u_material;
uniform DirectionalLight u_sun;
uniform vec3 u_cameraPosition = vec3(0.f);
uniform float u_ambience = 0.f;
uniform float u_bloomThreshold = 0.f;

const float k_gamma = 2.2f;

float CalculateDiffuse(vec3 N, vec3 lightDirection)
{
    const float diffuseFactor = max(dot(N, lightDirection), 0.f);
    return diffuseFactor;
}

float CalculateSpecular(vec3 N, vec3 lightDirection)
{
    const vec3 viewDirection = normalize(u_cameraPosition - v_input.position);
    const vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    const float shininess = 32.f;
    const float specularFactor = pow(max(dot(N, halfwayDirection), 0.f), shininess);
    return specularFactor;
}

vec4 GetColorAlbedo()
{
    const vec4 albedoTextureColor = pow(texture(u_material.albedoTexture, v_input.texCoord), vec4(vec3(k_gamma), 1.f));
    const vec4 albedo = u_material.albedo * albedoTextureColor;
    return albedo;
}

void main()
{
    const vec3 N = normalize(v_input.normal);
    const vec3 ambient = vec3(u_ambience);
    const vec4 albedo = GetColorAlbedo();
    const vec3 lightDirection = normalize(-u_sun.direction); 

    const float diffuseFactor = CalculateDiffuse(N, lightDirection);
    const vec3 diffuse = diffuseFactor * u_sun.color;

    const float specularFactor = CalculateSpecular(N, lightDirection);
    const vec3 specular = specularFactor * u_sun.color;

    const vec3 lighting = ambient + (diffuse + specular) * u_sun.intensity;
    finalColor = vec4(lighting, 1.f) * albedo;
    entityID = v_entityID;
    
    const vec3 directContribution = (diffuse + specular) * u_sun.intensity * albedo.rgb;
    const float brightness = dot(directContribution, vec3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > u_bloomThreshold)
        bloomColor = vec4(finalColor.rgb, 1.f);
    else
        bloomColor = vec4(0.f, 0.f, 0.f, 1.f);
}*/
