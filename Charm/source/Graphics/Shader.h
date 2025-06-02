#pragma once
#include "Core/Base.h"

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        class Shader
        {
        public:
            Shader();
            ~Shader();

            void Bind() const;
            void Unbind() const;
            void Load(const char* vertexPath, const char* fragmentPath);
            void CreateUniform(const char* name);
            void SetUniform(const char* name, u32 value);
            void SetUniform(const char* name, float value);
            void SetUniform(const char* name, const glm::vec3& value);
            void SetUniform(const char* name, const glm::vec4& value);
            void SetUniform(const char* name, const glm::mat4& value);
            void SetUniform(const char* name, s32* values, u32 count);

        private:
            u32 Compile(u32 type, const char* source);
            void Link(u32 vertexShader, u32 fragmentShader);

        private:
            u32 m_ID = 0;
            std::unordered_map<std::string, s32> m_UniformLocations;
        };

    }
}
