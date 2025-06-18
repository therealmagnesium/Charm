#version 450 core

in VERTEX_DATA
{
    vec3 localPosition;
    vec3 color;
    float thickness;
    float fade;
} data;

out vec4 finalColor;

void main()
{
    float d = 1.f - length(data.localPosition);
    float alpha = smoothstep(0.f, data.fade, d);
    alpha *= smoothstep(data.thickness + data.fade, data.thickness, d);

    if (alpha == 0.f)
        discard;

    vec3 result = data.color;
    result = pow(result, vec3(1.f / 2.2f));

    finalColor = vec4(result, 1.f);
    finalColor.a *= alpha;
}

