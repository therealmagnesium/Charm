#pragma once
#include "Core/Base.h"
#include "Core/Asset.h"

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        enum class TextureFormat : u8
        {
            None = 0,
            RGB,
            RGBA,
            Depth,
            DepthStencil,
            RedInteger
        };

        struct Texture : public Asset
        {
            u32 id = 0;
            u32 width = 0;
            u32 height = 0;
            u32 internalFormat = 0;
            u32 dataFormat = 0;
            u32 channelCount = 0;

            inline bool operator==(const Texture& other) const { return id == other.id; }
            inline AssetType GetType() override { return AssetType::Texture; }
        };

        namespace Textures
        {
            Texture Load(const char* path);
            Texture LoadEmpty(u32 width, u32 height, TextureFormat format);
            Texture LoadDefaultWhite();
            void Unload(Texture& texture);
            void Bind(const Texture& texture, u8 slot);
        }
    }
}
