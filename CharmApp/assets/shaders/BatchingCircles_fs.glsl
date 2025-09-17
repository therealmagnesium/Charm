#version 450 core

struct VertexData
{
    vec3 localPosition;
    vec3 color;
    float thickness;
    float fade;
};

layout (location = 0) in VertexData data;
layout (location = 4) in flat int v_entityID;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out int entityID;

void main()
{
    float d = 1.f - length(data.localPosition);
    float alpha = smoothstep(0.f, data.fade, d);
    alpha *= smoothstep(data.thickness + data.fade, data.thickness, d);

    if (alpha == 0.f)
        discard;

    vec3 result = data.color;
    //result = pow(result, vec3(1.f / 2.2f));

    finalColor = vec4(result, 1.f);
    finalColor.a *= alpha;

    entityID = v_entityID;
}

