#version 450 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texCoord;
layout (location = 2) in vec4 a_color;
layout (location = 0) out vec2 v_texCoord;

uniform mat4 u_matrixProjection;
uniform bool u_shouldBlur;

void main()
{
    if (!u_shouldBlur)
    {
        v_texCoord = a_texCoord;
        gl_Position = u_matrixProjection * vec4(a_position, 0.f, 1.f);
    }
    else
    {
        const vec3 vertices[4] = vec3[4](
            vec3(-1.f, 1.f, 0.f),
            vec3(-1.f, -1.f, 0.f),
            vec3(1.f, -1.f, 0.f),
            vec3(1.f, 1.f, 0.f)
        );

        const int indices[6] = int[6](0, 1, 2, 2, 3, 0);

        const vec2 texCoords[4] = vec2[4](
            vec2(0.f, 1.f),
            vec2(0.f, 0.f),
            vec2(1.f, 0.f),
            vec2(1.f, 1.f)
        );

        int index = indices[gl_VertexID];
        vec3 vertex = vertices[index];
        vec2 texCoord = texCoords[index];
        vec4 position = vec4(vertex, 1.f);

        v_texCoord = texCoord;
        gl_Position = position;
    }
}
