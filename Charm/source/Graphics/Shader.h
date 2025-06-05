#pragma once
#include "Core/Base.h"

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Shader
        {
            u32 id = 0;
            std::unordered_map<std::string, s32> uniformLocations;
        };

        namespace Shaders
        {
            void Bind(const Shader& shader);
            void Unbind();
            Shader Load(const char* vertexPath, const char* fragmentPath);
            void Unload(Shader& shader);
            void CreateUniform(Shader& shader, const char* name);
            void SetUniform(Shader& shader, const char* name, u32 value);
            void SetUniform(Shader& shader, const char* name, float value);
            void SetUniform(Shader& shader, const char* name, const glm::vec3& value);
            void SetUniform(Shader& shader, const char* name, const glm::vec4& value);
            void SetUniform(Shader& shader, const char* name, const glm::mat4& value);
            void SetUniform(Shader& shader, const char* name, s32* values, u32 count);
        }
    };
}
