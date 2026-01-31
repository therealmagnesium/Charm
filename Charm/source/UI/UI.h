#pragma once
#include "Core/Base.h"
#include <glm/glm.hpp>
#include <filesystem>

namespace Charm
{
    namespace Graphics
    {
        struct Animation;
        struct AnimationController;
        struct Texture;
        struct TilePalette;
    }

    namespace UI
    {
        void SetupContext();
        void DestroyContext();
        void HandleEvents(void* event);
        void BeginFrome();
        void EndFrame();
        void Display();

        bool DrawBoolControl(const char* label, bool* b, float columnWidth = 100.f);
        void DrawFloatControl(const char* label, float* v, float min, float max, float speed = 0.01f, float columnWidth = 100.f);
        void DrawVec2Control(const char* label, glm::vec2& v, float speed, float resetValue, float columnWidth = 100.f);
        void DrawVec3Control(const char* label, glm::vec3& v, float speed, float resetValue, float columnWidth = 100.f);
        void DrawColorControl(const char* label, glm::vec3& v, float columnWidth = 100.f);
        void DrawColorControl(const char* label, glm::vec4& v, float columnWidth = 100.f);
        void DrawIntInputControl(const char* label, s32* v, s32 min, s32 max, float columnWidth = 100.f);
        void DrawTextInputControl(const char* label, std::string* s, u32 flags = 0, float columnWidth = 100.f);
        bool DrawFilesystemInputControl(const char* label, std::filesystem::path* p, u32 flags = 0, float columnWidth = 100.f);
        void DrawAssetControls_Animation(Graphics::Animation* animation);
        void DrawAssetControls_AnimationController(Graphics::AnimationController* controller);
        void DrawAssetControls_Texture(Graphics::Texture* texture);
        void DrawAssetControls_TilePalette(Graphics::TilePalette* tilePalette);
    }
}
