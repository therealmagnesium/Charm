#pragma once
#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Charm
{
    namespace Utils
    {
        inline std::string GetFileName(const char* path, bool hasExtension = false)
        {
            std::filesystem::path p(path);
            std::string fileName = (!hasExtension) ? p.stem().string() : p.filename().string();
            return fileName;
        }

        inline glm::mat4 GetTransfomMatrix2D(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin)
        {
            glm::mat4 transform = glm::mat4(1.f);
            transform = glm::translate(transform, glm::vec3(position, 0.f));
            transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
            transform = glm::translate(transform, glm::vec3(-origin, 0.f));
            transform = glm::scale(transform, glm::vec3(size, 1.f));

            return transform;
        }
    }
}
