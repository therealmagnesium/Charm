#version 450

struct VertexData
{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec3 normal;
};

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texCoord;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in mat4 a_instanceTransform;
layout (location = 8) in int a_instanceEntityID;

layout (location = 0) out VertexData v_output;
layout (location = 4) out flat int v_entityID;

uniform mat4 u_matrixView;
uniform mat4 u_matrixProjection;

void main()
{
    const vec4 worldPosition = a_instanceTransform * vec4(a_position, 1.f);
    const mat3 matrixNormal = mat3(transpose(inverse(a_instanceTransform)));

    v_output.position = worldPosition.xyz;
    v_output.color = a_color;
    v_output.texCoord = a_texCoord;
    v_output.normal = matrixNormal * a_normal;

    v_entityID = a_instanceEntityID;
    gl_Position = u_matrixProjection * u_matrixView * worldPosition;
}

