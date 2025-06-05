#include "Graphics/Shader.h"

#include "Core/Base.h"
#include "Core/IO.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        const static u32 k_ReservedUniformCount = 10;

        namespace Shaders
        {
            u32 Compile(u32 type, const char* source);
            void Link(Shader& shader, u32 vertexShader, u32 fragmentShader);

            Shader Load(const char* vertexPath, const char* fragmentPath)
            {
                Shader shader;
                shader.id = glCreateProgram();
                shader.uniformLocations.reserve(k_ReservedUniformCount);

                File vertexFile = IO::ReadFile(vertexPath);
                File fragmentFile = IO::ReadFile(fragmentPath);

                if (!vertexFile.isValid || !fragmentFile.isValid)
                {
                    WARN("Failed to load shader with an ID of %d", shader.id);
                    return shader;
                }

                const char* vertexSource = vertexFile.asCString();
                const char* fragmentSource = fragmentFile.asCString();

                u32 vertexShader = Compile(GL_VERTEX_SHADER, vertexSource);
                u32 fragmentShader = Compile(GL_FRAGMENT_SHADER, fragmentSource);

                Link(shader, vertexShader, fragmentShader);

                std::string vertexShaderName = Utils::GetFileName(vertexPath);
                std::string fragmentShaderName = Utils::GetFileName(fragmentPath);
                INFO("Shader [\"%s\", \"%s\"] was successfully created with an ID of %d",
                     vertexShaderName.c_str(), fragmentShaderName.c_str(), shader.id);

                return shader;
            }

            void Unload(Shader& shader)
            {
                INFO("Unloading shader with an ID of %d...", shader.id);
                glDeleteProgram(shader.id);
                shader.id = 0;
            }

            void Bind(const Shader& shader)
            {
                glUseProgram(shader.id);
            }

            void Unbind()
            {
                glUseProgram(0);
            }

            void CreateUniform(Shader& shader, const char* name)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                {
                    WARN("Shader with id %d already has uniform \"%s\"", shader.id, name);
                    return;
                }

                shader.uniformLocations[name] = glGetUniformLocation(shader.id, name);

                if (shader.uniformLocations[name] == -1)
                    WARN("Shader with id %d could not find uniform \"%s\"", shader.id, name);
            }

            void SetUniform(Shader& shader, const char* name, u32 value)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniform1i(shader.uniformLocations[name], value);
            }

            void SetUniform(Shader& shader, const char* name, float value)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniform1f(shader.uniformLocations[name], value);
            }

            void SetUniform(Shader& shader, const char* name, const glm::vec3& value)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniform3fv(shader.uniformLocations[name], 1, glm::value_ptr(value));
            }

            void SetUniform(Shader& shader, const char* name, const glm::vec4& value)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniform4fv(shader.uniformLocations[name], 1, glm::value_ptr(value));
            }

            void SetUniform(Shader& shader, const char* name, const glm::mat4& value)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniformMatrix4fv(shader.uniformLocations[name], 1, false, glm::value_ptr(value));
            }

            void SetUniform(Shader& shader, const char* name, s32* values, u32 count)
            {
                if (shader.uniformLocations.find(name) != shader.uniformLocations.end())
                    glUniform1iv(shader.uniformLocations[name], count, values);
            }

            u32 Compile(u32 type, const char* source)
            {
                u32 shader = glCreateShader(type);
                glShaderSource(shader, 1, &source, NULL);
                glCompileShader(shader);

                s32 success = 0;
                char infoLog[512];

                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 512, NULL, infoLog);
                    ERROR("Failed to compile shader! \n%s", infoLog);
                };

                return shader;
            }

            void Link(Shader& shader, u32 vertexShader, u32 fragmentShader)
            {
                glAttachShader(shader.id, vertexShader);
                glAttachShader(shader.id, fragmentShader);
                glLinkProgram(shader.id);
                glValidateProgram(shader.id);

                s32 success = 0;
                char infoLog[512];

                glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader.id, 512, NULL, infoLog);
                    ERROR("Failed to link shader! \n%s", infoLog);
                };

                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
            }
        }
    }
}
