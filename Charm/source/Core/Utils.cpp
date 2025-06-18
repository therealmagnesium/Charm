#include "Core/Utils.h"
#include "Core/Application.h"
#include "Core/Asset.h"
#include "Graphics/Window.h"

#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace Utils
    {
        std::string GetFileName(const char* path, bool hasExtension)
        {
            std::filesystem::path p(path);
            std::string fileName = (!hasExtension) ? p.stem().string() : p.filename().string();
            return fileName;
        }

        std::string AssetTypeToString(AssetType type)
        {
            std::string value = "Invalid";

            switch (type)
            {
                case AssetType::Texture:
                    value = "Texture";
                    break;

                case AssetType::Shader:
                    value = "Shader";
                    break;

                default:
                    break;
            }

            return value;
        }

        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition)
        {
            const ApplicationConfig& config = Application::GetConfig();
            const u32 windowWidth = Window::GetWidth();
            const u32 windowHeight = Window::GetHeight();

            glm::vec2 scale;
            scale.x = (float)config.virtualWidth / (float)windowWidth;
            scale.y = (float)config.virtualHeight / (float)windowHeight;

            glm::vec2 virtualPosition;
            virtualPosition.x = screenPosition.x * scale.x;
            virtualPosition.y = screenPosition.y * scale.y;

            return virtualPosition;
        }

        glm::mat4 GetTransfomMatrix2D(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin)
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
