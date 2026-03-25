#version 450 core

layout (location = 0) in vec2 v_texCoord;
layout (location = 0) out vec4 finalColor;

uniform float u_exposure = 1.f;
uniform bool u_isHorizontal;
uniform bool u_shouldBlur;
uniform sampler2D u_textureScreen;
uniform sampler2D u_textureBloom;

const float k_gamma = 2.2f;

vec3 ConvertToGrayscale(vec3 color)
{
    const float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec3(average);
}

vec3 GaussianBlur()
{
    const float weights[5] = float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);
    const vec2 offset = 1.f / textureSize(u_textureScreen, 0);
    vec3 result = texture(u_textureScreen, v_texCoord).rgb * weights[0];
    if (u_isHorizontal)
    {
        for (int i = 0; i < 5; i++)
        {
            result += texture(u_textureScreen, v_texCoord + vec2(offset.x * i, 0.f)).rgb * weights[i];
            result += texture(u_textureScreen, v_texCoord - vec2(offset.x * i, 0.f)).rgb * weights[i];
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            result += texture(u_textureScreen, v_texCoord + vec2(0.f, offset.y * i)).rgb * weights[i];
            result += texture(u_textureScreen, v_texCoord - vec2(0.f, offset.y * i)).rgb * weights[i];
        }
    }

    return result;
}

vec3 ToneMapHDR(vec3 color)
{
    return vec3(1.f) - exp(-color * u_exposure);
}

vec3 ApplyGammaCorrection(vec3 color)
{
    return pow(color, vec3(1.f / k_gamma));
}

void main()
{ 
    if (!u_shouldBlur)
    {
        finalColor.rgb = texture(u_textureScreen, v_texCoord).rgb + texture(u_textureBloom, v_texCoord).rgb;
        finalColor.rgb = ToneMapHDR(finalColor.rgb);
        finalColor.rgb = ApplyGammaCorrection(finalColor.rgb);
        finalColor.a = 1.f;
    }
    else
    {
        finalColor.rgb = GaussianBlur();
        finalColor.a = 1.f;
    }
}
