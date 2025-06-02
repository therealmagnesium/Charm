#include "Graphics/Texture.h"
#include "Core/Log.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Charm
{
    namespace Graphics
    {
        namespace Textures
        {
            Texture Load(const char* path)
            {
                Texture texture;

                u8* data = stbi_load(path, (s32*)&texture.width, (s32*)&texture.height, (s32*)&texture.channelCount, 0);
                if (data == NULL)
                {
                    WARN("Failed to load texture \"%s\"", path, texture.id);
                    return texture;
                }

                switch (texture.channelCount)
                {
                    case 3:
                        texture.internalFormat = GL_SRGB8;
                        texture.dataFormat = GL_RGB;
                        break;

                    case 4:
                        texture.internalFormat = GL_SRGB8_ALPHA8;
                        texture.dataFormat = GL_RGBA;
                        break;

                    default:
                        break;
                }

                glGenTextures(1, &texture.id);
                glBindTexture(GL_TEXTURE_2D, texture.id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat, texture.width,
                             texture.height, 0, texture.dataFormat, GL_UNSIGNED_BYTE, data);
                glBindTexture(GL_TEXTURE_2D, 0);

                stbi_image_free(data);

                INFO("Texture \"%s\" was loaded successfully with an ID of %d", path, texture.id);
                return texture;
            }

            Texture LoadDefaultWhite()
            {
                Texture texture;
                texture.width = 1;
                texture.height = 1;
                texture.internalFormat = GL_RGBA8;
                texture.dataFormat = GL_RGBA;
                texture.channelCount = 4;

                glGenTextures(1, &texture.id);
                glBindTexture(GL_TEXTURE_2D, texture.id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                u32 color = 0xFFFFFFFF;
                glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat, texture.width,
                             texture.height, 0, texture.dataFormat, GL_UNSIGNED_BYTE, &color);
                glBindTexture(GL_TEXTURE_2D, 0);

                INFO("Default white texture was loaded successfully with an ID of %d", texture.id);
                return texture;
            }

            void Unload(Texture& texture)
            {
                if (texture.id != 0)
                {
                    INFO("Unloading texture with an ID of %d...", texture.id);
                    glDeleteTextures(1, &texture.id);
                }
            }

            void Bind(const Texture& texture, u8 slot)
            {
                glBindTextureUnit(slot, texture.id);
            }
        }
    }
}
