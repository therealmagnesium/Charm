#pragma once
#include "Core/Base.h"
#include "Graphics/Texture.h"

#include <vector>

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
            void Destroy(Framebuffer& framebuffer);
        }
    }
}
