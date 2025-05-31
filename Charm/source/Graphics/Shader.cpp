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
        static const u32 reservedUniformCount = 10;

        Shader::Shader()
        {
            m_uniformLocations.reserve(reservedUniformCount);
        }

        Shader::~Shader()
        {
            glDeleteProgram(m_id);
            m_id = 0;
        }

        void Shader::Load(const char* vertexPath, const char* fragmentPath)
        {
            File vertexFile = IO::ReadFile(vertexPath);
            File fragmentFile = IO::ReadFile(fragmentPath);

            if (!vertexFile.isValid || !fragmentFile.isValid)
            {
                WARN("Failed to load shader with an ID of %d", m_id);
                return;
            }

            const char* vertexSource = vertexFile.asCString();
            const char* fragmentSource = fragmentFile.asCString();

            u32 vertexShader = Shader::Compile(GL_VERTEX_SHADER, vertexSource);
            u32 fragmentShader = Shader::Compile(GL_FRAGMENT_SHADER, fragmentSource);

            Shader::Link(vertexShader, fragmentShader);

            std::string vertexShaderName = Utils::GetFileName(vertexPath);
            std::string fragmentShaderName = Utils::GetFileName(fragmentPath);
            INFO("Shader [\"%s\", \"%s\"] was successfully created with an ID of %d",
                 vertexShaderName.c_str(), fragmentShaderName.c_str(), m_id);
        }

        void Shader::Bind() const
        {
            glUseProgram(m_id);
        }

        void Shader::Unbind() const
        {
            glUseProgram(0);
        }

        void Shader::CreateUniform(const char* name)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
            {
                WARN("Shader with id %d already has uniform \"%s\"", m_id, name);
                return;
            }

            m_uniformLocations[name] = glGetUniformLocation(m_id, name);

            if (m_uniformLocations[name] == -1)
                WARN("Shader with id %d could not find uniform \"%s\"", m_id, name);
        }

        void Shader::SetUniform(const char* name, u32 value)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
                glUniform1i(m_uniformLocations[name], value);
        }

        void Shader::SetUniform(const char* name, float value)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
                glUniform1f(m_uniformLocations[name], value);
        }

        void Shader::SetUniform(const char* name, const glm::vec3& value)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
                glUniform3fv(m_uniformLocations[name], 1, glm::value_ptr(value));
        }

        void Shader::SetUniform(const char* name, const glm::vec4& value)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
                glUniform4fv(m_uniformLocations[name], 1, glm::value_ptr(value));
        }

        void Shader::SetUniform(const char* name, const glm::mat4& value)
        {
            if (m_uniformLocations.find(name) != m_uniformLocations.end())
                glUniformMatrix4fv(m_uniformLocations[name], 1, false, glm::value_ptr(value));
        }

        u32 Shader::Compile(u32 type, const char* source)
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

        void Shader::Link(u32 vertexShader, u32 fragmentShader)
        {
            m_id = glCreateProgram();
            glAttachShader(m_id, vertexShader);
            glAttachShader(m_id, fragmentShader);
            glLinkProgram(m_id);
            glValidateProgram(m_id);

            s32 success = 0;
            char infoLog[512];

            glGetProgramiv(m_id, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(m_id, 512, NULL, infoLog);
                ERROR("Failed to link shader! \n%s", infoLog);
            };

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
    }
}
