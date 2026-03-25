#version 450 core

// Intentionally empty. OpenGL writes depth automatically from gl_Position.z.
// No colour attachment is bound for shadow pass FBOs, so no output is needed.
void main()
{
}
