#include "Graphics/Framebuffer.h"
#include "Graphics/RenderCommand.h"

#include "Core/Log.h"

#include <glad/glad.h>

namespace Charm
{
    namespace Graphics
    {
        namespace Framebuffers
        {
            Framebuffer Create(const FramebufferSpecification& spec)
            {
                Framebuffer framebuffer;
                framebuffer.attachments.resize(spec.numAttachments);
                framebuffer.specification = spec;

                glGenFramebuffers(1, &framebuffer.id);
                Invalidate(framebuffer);

                return framebuffer;
            }

            void Invalidate(Framebuffer& framebuffer)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);

                framebuffer.attachments[0] = Textures::LoadEmpty(framebuffer.specification.width,
                                                                 framebuffer.specification.height,
                                                                 TextureFormat::RGBA);
                Textures::Bind(framebuffer.attachments[0], 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.attachments[0].id, 0);

                if (framebuffer.specification.numAttachments > 1)
                {
                    framebuffer.attachments[1] = Textures::LoadEmpty(framebuffer.specification.width,
                                                                     framebuffer.specification.height,
                                                                     TextureFormat::DepthStencil);
                    Textures::Bind(framebuffer.attachments[1], 0);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer.attachments[1].id, 0);
                }

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    ERROR("Failed to validate framebuffer with an ID of %d!", framebuffer.id);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            void Bind(Framebuffer& framebuffer)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
                RenderCommand::SetViewport(0, 0, framebuffer.specification.width, framebuffer.specification.height);
            }

            void Unbind()
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            void Destroy(Framebuffer& framebuffer)
            {
                if (framebuffer.id != 0)
                {
                    INFO("Unloading framebuffer with an ID of %d...", framebuffer.id);

                    for (Texture& attachment : framebuffer.attachments)
                        Textures::Unload(attachment);

                    framebuffer.attachments.clear();
                    glDeleteFramebuffers(1, &framebuffer.id);
                }
            }
        }
    }
}
