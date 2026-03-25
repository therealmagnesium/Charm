#version 450 core

// Matches the instance buffer layout set up by Meshes::SetupInstanceBuffer.
// The shadow pass only needs position and the per-instance world transform —
// no normals, colours, or texture coordinates are required.
layout (location = 0) in vec3 a_position;
layout (location = 4) in mat4 a_instanceTransform;   // locations 4-7

uniform mat4 u_lightSpaceMatrix;

void main()
{
    gl_Position = u_lightSpaceMatrix * a_instanceTransform * vec4(a_position, 1.0);
}
