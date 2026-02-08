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

layout (location = 0) out VertexData v_output;

uniform mat4 u_matrixTransform;
uniform mat4 u_matrixView;
uniform mat4 u_matrixProjection;
uniform mat4 u_matrixNormal;

void main()
{
    const vec4 worldPosition = u_matrixTransform * vec4(a_position, 1.f);
    v_output.position = worldPosition.xyz;
    v_output.color = a_color;
    v_output.texCoord = a_texCoord;
    v_output.normal = mat3(u_matrixNormal) * a_normal;

    gl_Position = u_matrixProjection * u_matrixView * worldPosition;
}
