#version 450 core

layout (location = 0) in vec2 v_texCoords;
layout (location = 0) out vec4 finalColor;

uniform float u_cameraZoom = 1.f;
uniform vec2 u_cameraPosition = vec2(0.f);
uniform vec2 u_resolution = vec2(640.f, 360.f);
uniform uint u_pixelsPerUnit = 32;
uniform uint u_tileScale = 1;

const vec4 k_colorLight = vec4(0.18f, 0.18f, 0.18f, 1.f);
const vec4 k_colorDark = vec4(0.f);

/*
vec3 CheckerBoard(vec2 coord, vec2 size)
{
    vec2 p = floor(coord / size); 
    float m = mod(p.x + p.y, 2.0);
    return (m > 0.f) ? k_colorLight : k_colorDark;
}*/

float Grid(vec2 fragCoord, float space, float gridWidth)
{
    vec2 p = fragCoord;
    vec2 size = vec2(gridWidth);
    
    vec2 a1 = mod(p - size, space);
    vec2 a2 = mod(p + size, space);
    vec2 a = a2 - a1;
       
    float g = min(a.x, a.y);
    return clamp(g, 0., 1.0);
}

void main()
{
    const float pixelsPerUnit = u_pixelsPerUnit * u_cameraZoom * 2.f;

    vec2 fragCoord = (v_texCoords - 0.5f) * 2.f;
    fragCoord *= u_resolution;
    fragCoord += u_cameraPosition * pixelsPerUnit;

    const float cellSpacing = float(pixelsPerUnit * u_tileScale);
    const float lineWidthThick = clamp(4.f * u_cameraZoom, 4.f, 8.f);
    const float lineWidthThin = clamp(2.f * u_cameraZoom, 1.f, 2.f);
    const float g1 = Grid(fragCoord, cellSpacing, lineWidthThick);
    const float g2 = (u_cameraZoom > 1.8f) ? Grid(fragCoord, cellSpacing / 4.f, lineWidthThin) : 1.f;
    const float g = g1 * g2; 

    const vec4 c = g > 0.f ? k_colorDark : k_colorLight; 
    finalColor = c;
 
    /*
    // Gradient across screen
    vec2 p = fragCoord;           // Point
	vec2 c = resolution.xy / 2.0;       // Center
    col *= (1.0 - length(c - p) / resolution.x * 0.7);*/	

    //col *= clamp(grid(gl_FragCoord.xy, 10.f, 0.5) * grid(gl_FragCoord.xy, 50.f, 1.f), 0.7f, 1.0);

    /*
    float pixelsPerUnit = 128.f * v_cameraZoom;
    vec2 coord = v_texCoords * resolution + v_cameraPosition * pixelsPerUnit;
    vec3 c = CheckerBoard(coord, vec2(pixelsPerUnit));
    finalColor = vec4(c, 1.f);*/
}
