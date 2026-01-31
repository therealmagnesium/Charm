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

            inline bool operator==(const Shader& other) { return id == other.id; }
            inline bool operator!=(const Shader& other) { return id != other.id; }
        };

        inline const Shader Shader_Invalid;

        namespace Shaders
        {
            void Bind(const Shader& shader);
            void Unbind();
            Shader Load(const char* vertexPath, const char* fragmentPath);
            void Unload(Shader& shader);
            void CreateUniform(Shader& shader, const char* name);
            void SetUniform(Shader& shader, const char* name, s32 value);
            void SetUniform(Shader& shader, const char* name, u32 value);
            void SetUniform(Shader& shader, const char* name, float value);
            void SetUniform(Shader& shader, const char* name, const glm::ivec2& value);
            void SetUniform(Shader& shader, const char* name, const glm::vec2& value);
            void SetUniform(Shader& shader, const char* name, const glm::vec3& value);
            void SetUniform(Shader& shader, const char* name, const glm::vec4& value);
            void SetUniform(Shader& shader, const char* name, const glm::mat4& value);
            void SetUniform(Shader& shader, const char* name, s32* values, u32 count);
        }
    };
}
