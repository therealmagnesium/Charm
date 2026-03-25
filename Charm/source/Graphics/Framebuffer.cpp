#include "Graphics/Framebuffer.h"
#include "Graphics/RenderAPI.h"

#include "Core/Log.h"
#include "Core/Utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Charm::Graphics
{
    namespace Framebuffers
    {
        Framebuffer Create(const FramebufferSpecification& spec)
        {
            Framebuffer framebuffer;
            framebuffer.specification = spec;
            framebuffer.colorAttachments.reserve(framebuffer.colorAttachmentSpecifications.size());

            for (FramebufferTextureSpecification& spec : framebuffer.specification.attachments.attachments)
            {
                if (!Utils::IsDepthFormat(spec.format))
                    framebuffer.colorAttachmentSpecifications.emplace_back(spec);
                else
                    framebuffer.depthAttachmentSpecification = spec;
            }

            glGenFramebuffers(1, &framebuffer.id);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
            Invalidate(framebuffer);

            INFO("Framebuffer was created successfully with an ID of %d", framebuffer.id);
            return framebuffer;
        }

        void Bind(Framebuffer& framebuffer)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
            RenderAPI::SetViewport(0, 0, framebuffer.specification.width, framebuffer.specification.height);
        }

        void Unbind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void Invalidate(Framebuffer& framebuffer)
        {
            for (auto& spec : framebuffer.colorAttachmentSpecifications)
            {
                Texture blank = Textures::LoadEmpty(framebuffer.specification.width,
                                                    framebuffer.specification.height,
                                                    spec.format);

                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0 + framebuffer.colorAttachments.size(),
                                       GL_TEXTURE_2D, blank.id, 0);

                framebuffer.colorAttachments.emplace_back(blank);
            }

            if (framebuffer.depthAttachmentSpecification.format != TextureFormat::None)
            {
                framebuffer.depthAttachment = Textures::LoadEmpty(framebuffer.specification.width,
                                                                  framebuffer.specification.height,
                                                                  framebuffer.depthAttachmentSpecification.format);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                       GL_TEXTURE_2D, framebuffer.depthAttachment.id, 0);
            }

            if (framebuffer.colorAttachments.size() > 0)
            {
                ASSERT(framebuffer.colorAttachments.size() <= 4, "Framebuffers::Invalidate - Framebuffers currently support only 4 color attachments!");

                u32 buffers[4];
                for (u32 i = 0; i < LEN(buffers); i++)
                    buffers[i] = GL_COLOR_ATTACHMENT0 + i;

                glDrawBuffers(framebuffer.colorAttachments.size(), buffers);
            }
            else
                glDrawBuffer(GL_NONE);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                ERROR("Failed to validate framebuffer with an ID of %d!", framebuffer.id);
        }

        s32 ReadPixel(Framebuffer& framebuffer, u32 attachmentIndex, u32 x, u32 y)
        {
            s32 pixelData = -1;
            ASSERT_RETURN(attachmentIndex < framebuffer.colorAttachments.size(), pixelData, "Framebuffers::ReadPixel - Invalid attachment index!");

            glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
            glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

            return pixelData;
        }

        glm::vec4 ReadPixelColor(Framebuffer& framebuffer, u32 attachmentIndex, u32 x, u32 y)
        {
            glm::vec4 pixelColor;
            ASSERT_RETURN(attachmentIndex < framebuffer.colorAttachments.size(), pixelColor, "Framebuffers::ReadPixel - Invalid attachment index!");

            glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, glm::value_ptr(pixelColor));

            return pixelColor;
        }

        void ClearAttachment(Framebuffer& framebuffer, u32 attachmentIndex, int value)
        {
            ASSERT(attachmentIndex < framebuffer.colorAttachments.size(), "Framebuffers::ClearAttachment - Invalid attachment index!");

            Texture& attachment = framebuffer.colorAttachments[attachmentIndex];
            glClearTexImage(attachment.id, 0, attachment.dataFormat, GL_INT, &value);
        }

        void Destroy(Framebuffer& framebuffer)
        {
            if (framebuffer.id != 0)
            {
                INFO("Framebuffer with an ID of %d is unloading...", framebuffer.id);

                for (Texture& attachment : framebuffer.colorAttachments)
                    Textures::Unload(attachment);

                Textures::Unload(framebuffer.depthAttachment);

                framebuffer.colorAttachments.clear();
                framebuffer.colorAttachmentSpecifications.clear();

                glDeleteFramebuffers(1, &framebuffer.id);
            }
        }

        u32 GetColorAttachmentWidth(const Framebuffer& framebuffer, u32 attachmentIndex)
        {
            u32 width = 0;
            if (attachmentIndex < framebuffer.colorAttachments.size())
            {
                const Texture& attachment = framebuffer.colorAttachments[attachmentIndex];
                if (attachment.isValid)
                    width = attachment.width;
            }

            return width;
        }

        u32 GetColorAttachmentHeight(const Framebuffer& framebuffer, u32 attachmentIndex)
        {
            u32 height = 0;
            if (attachmentIndex < framebuffer.colorAttachments.size())
            {
                const Texture& attachment = framebuffer.colorAttachments[attachmentIndex];
                if (attachment.isValid)
                    height = attachment.height;
            }

            return height;
        }

        u32 GetDepthAttachmentWidth(const Framebuffer& framebuffer) { return framebuffer.depthAttachment.isValid ? framebuffer.depthAttachment.width : 0; }
        u32 GetDepthAttachmentHeight(const Framebuffer& framebuffer) { return framebuffer.depthAttachment.isValid ? framebuffer.depthAttachment.height : 0; }
    }
}
