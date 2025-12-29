#pragma once
#include "Core/Base.h"
#include "Core/Asset.h"

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
            RedInteger,
            _TotalCount
        };

        enum class TextureFilter : u8
        {
            Linear = 0,
            Nearest,
            LinearMipmapLinear,
            LinearMipmapNearest,
            NearestMipmapLinear,
            NearestMipmapNearest,
            _TotalCount,
        };

        enum class TextureMode : u8
        {
            Single = 0,
            SpriteSheet,
            Tileset,
            _TotalCount
        };

        struct Texture : public Core::Asset
        {
            u32 id = 0;
            u32 width = 0;
            u32 height = 0;
            u32 channelCount = 0;
            u32 internalFormat = 0;
            u32 dataFormat = 0;
            TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
            TextureFilter magFilter = TextureFilter::Linear;
            u8* data = NULL;

            inline bool operator==(const Texture& other) const { return id == other.id; }
            inline Core::AssetType GetType() const override { return Core::AssetType::Texture; }
        };

        namespace Textures
        {
            Texture Load(const char* path, TextureFilter minFilter = TextureFilter::LinearMipmapLinear, TextureFilter magFilter = TextureFilter::Linear);
            Texture LoadEmpty(u32 width, u32 height, TextureFormat format);
            Texture LoadDefaultWhite();
            void Unload(Texture& texture);
            void Bind(const Texture& texture, u8 slot);
            void Invalidate(Texture& texture);
        }
    }
}
