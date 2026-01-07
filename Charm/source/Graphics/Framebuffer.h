#pragma once
#include "Core/Base.h"
#include "Graphics/Texture.h"

#include <vector>
#include <glm/vec4.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct FramebufferTextureSpecification
        {
            TextureFormat format = TextureFormat::None;

            FramebufferTextureSpecification() = default;
            FramebufferTextureSpecification(TextureFormat format) : format(format) {}
        };

        struct FramebufferAttachmentSpecification
        {
            std::vector<FramebufferTextureSpecification> attachments;

            FramebufferAttachmentSpecification() = default;
            FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : attachments(attachments) {};
        };

        struct FramebufferSpecification
        {
            u32 width = 0;
            u32 height = 0;
            u32 numSamples = 1;
            bool swapChainTarget = false;
            FramebufferAttachmentSpecification attachments;
        };

        struct Framebuffer
        {
            u32 id = 0;
            FramebufferSpecification specification;

            std::vector<FramebufferTextureSpecification> colorAttachmentSpecifications;
            FramebufferTextureSpecification depthAttachmentSpecification;

            std::vector<Texture> colorAttachments;
            Texture depthAttachment;
        };

        namespace Framebuffers
        {
            Framebuffer Create(const FramebufferSpecification& spec);
            void Bind(Framebuffer& framebuffer);
            void Unbind();
            void Invalidate(Framebuffer& framebuffer);
            s32 ReadPixel(Framebuffer& framebuffer, u32 attachmentIndex, u32 x, u32 y);
            glm::vec4 ReadPixelColor(Framebuffer& framebuffer, u32 attachmentIndex, u32 x, u32 y);
            void ClearAttachment(Framebuffer& framebuffer, u32 attachmentIndex, int value);
            void Destroy(Framebuffer& framebuffer);

            u32 GetColorAttachmentWidth(const Framebuffer& framebuffer, u32 attachmentIndex = 0);
            u32 GetColorAttachmentHeight(const Framebuffer& framebuffer, u32 attachmentIndex = 0);
            u32 GetDepthAttachmentWidth(const Framebuffer& framebuffer);
            u32 GetDepthAttachmentHeight(const Framebuffer& framebuffer);
        }
    }
}
