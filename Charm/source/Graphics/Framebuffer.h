#pragma once
#include "Core/Base.h"
#include "Graphics/Texture.h"

#include <vector>

namespace Charm
{
    namespace Graphics
    {
        struct FramebufferSpecification
        {
            u32 width = 0;
            u32 height = 0;
            u32 numSamples = 1;
            u32 numAttachments = 1;
            bool swapChainTarget = false;
        };

        struct Framebuffer
        {
            u32 id = 0;
            std::vector<Texture> attachments;
            FramebufferSpecification specification;
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
