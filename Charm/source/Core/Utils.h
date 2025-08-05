#pragma once
#include "Graphics/Shapes.h"
#include "Graphics/Texture.h"
#include "ECS/PhysicsWorld.h"

#include <string>
#include <glm/glm.hpp>

struct b2BodyId;
struct b2ShapeId;
struct b2WorldId;
struct b2WorldDef;

namespace Charm
{
    namespace Utils
    {
        const char* BoolToCString(bool value);
        bool IsDepthFormat(Graphics::TextureFormat format);
        u32 TextureFilterToGL(Graphics::TextureFilter filter);
        std::string TextureFilterToString(Graphics::TextureFilter filter);
        Graphics::TextureFilter StringToTextureFilter(const std::string& str);
        std::string GetFileName(const char* path, bool hasExtension = false);
        std::string AssetTypeToString(Core::AssetType type);
        Core::AssetType StringToAssetType(const std::string& str);
        Core::AssetType ExtensionToAssetType(const std::string& extension);
        ECS::PhysicsBodyType StringToBodyType(const std::string& str);
        std::string BodyTypeToString(ECS::PhysicsBodyType type);
        u32 BodyTypeToB2BodyType(ECS::PhysicsBodyType type);
        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition);
        glm::vec2 ScreenToViewport(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize);
        glm::vec2 ScreenToViewportGL(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize);
        glm::mat4 GetTransfomMatrix2D(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec2& origin);
        glm::vec2 OriginModeToVec2(Graphics::OriginMode mode, const glm::vec2& position, const glm::vec2& size);
        std::string OriginModeToString(Graphics::OriginMode mode);
        Graphics::OriginMode StringToOriginMode(const std::string& str);
        ECS::PhysicsBodyID B2BodyToPhysicsBody(b2BodyId& body);
        ECS::PhysicsShapeID B2ShapeToPhysicsShape(b2ShapeId& shape);
        ECS::PhysicsWorldID B2WorldToPhysicsWorldID(b2WorldId& world);
        ECS::PhysicsWorld B2WorldDefToPhysicsWorld(b2WorldDef& worldDef);
    }
}
