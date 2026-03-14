#include <Graphics/Lights.h>
#include <Graphics/Shader.h>
#include <Graphics/Renderer.h>
#include <glm/vec3.hpp>

namespace Charm::Graphics
{
    namespace Lights
    {
        void UpdateUniforms(const DirectionalLight& sun)
        {
            Shader& validShader = sun.shader != NULL ? *sun.shader : Renderer::GetShaderBlinnPhong();

            Shaders::Bind(validShader);
            Shaders::SetUniform(validShader, "u_sun.direction", sun.direction);
            Shaders::SetUniform(validShader, "u_sun.color", sun.color);
            Shaders::SetUniform(validShader, "u_sun.intensity", sun.intensity);
            Shaders::Unbind();
        }
    }
}
