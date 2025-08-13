#include "Graphics/Animation.h"
#include "Core/AssetManager.h"
#include "Core/Log.h"
#include "ECS/Components.h"
#include "Core/Utils.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        namespace Animations
        {
            Animation Load(const char* path)
            {
                Animation animation;
                YAML::Node data = YAML::LoadFile(path);
                ASSERT_RETURN(data, animation, "Animations::Load - Failed to load animation %s, the path may be invalid!", path);
                ASSERT_RETURN(data["Animation"], animation, "Animations::Load - Failed to load animation %s, the path may be an invalid animation file!", path);

                YAML::Node properties = data["Properties"];
                animation.speed = properties["Speed"].as<u32>();
                animation.frameCount = properties["Frame Count"].as<u32>();
                animation.rowCount = properties["Row Count"].as<u32>();
                animation.rowOffset = properties["Row Offset"].as<u32>();
                animation.columnCount = properties["Column Count"].as<u32>();
                animation.columnOffset = properties["Column Offset"].as<u32>();
                animation.shouldLoop = properties["Should Loop"].as<bool>();
                animation.spriteSheetType = Utils::StringToSpriteSheetAnimType(properties["Sprite Sheet Type"].as<std::string>());

                INFO("Animation \"%s\" was loaded successfully", path);
                return animation;
            }

            void Save(const char* path, const Animation& animation)
            {
                YAML::Emitter out;
                out << YAML::BeginMap;

                out << YAML::Key << "Animation" << YAML::Value << animation.handle;
                out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Speed" << YAML::Value << animation.speed;
                out << YAML::Key << "Frame Count" << YAML::Value << animation.frameCount;
                out << YAML::Key << "Row Count" << YAML::Value << animation.rowCount;
                out << YAML::Key << "Row Offset" << YAML::Value << animation.rowOffset;
                out << YAML::Key << "Column Count" << YAML::Value << animation.columnCount;
                out << YAML::Key << "Column Offset" << YAML::Value << animation.columnOffset;
                out << YAML::Key << "Should Loop" << YAML::Value << animation.shouldLoop;
                out << YAML::Key << "Sprite Sheet Type" << YAML::Value << Utils::SpriteSheetAnimTypeToString(animation.spriteSheetType);

                out << YAML::EndMap;

                out << YAML::EndMap;

                std::ofstream fout(path);
                if (fout.is_open())
                {
                    fout << out.c_str() << "\n";
                    fout.close();

                    INFO("Animation \"%s\" was saved successfully", path);
                }
                else
                    ERROR("Animations::Save - Failed to save animation %s, the path may be invalid!", path);
            }

            void Reset(Animation& animation)
            {
                animation.currentFrame = 0;
                animation.counter = 0;
            }

            void Update(Animation& animation, ECS::SpriteRendererComponent& spriteRenderer)
            {
                Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                ASSERT_ERROR(texture != NULL, "Animations::Update - Caannot update animation because the attached sprite renderer has no texture!");

                spriteRenderer.crop.width = (float)texture->width / animation.columnCount;
                spriteRenderer.crop.height = (float)texture->height / animation.rowCount;

                const u32 horizontalOffset = animation.columnOffset * spriteRenderer.crop.width;
                const u32 verticalOffset = animation.rowOffset * spriteRenderer.crop.height;

                animation.counter++;

                if (animation.counter >= (60 / animation.speed))
                {
                    animation.counter = 0;
                    animation.currentFrame++;

                    if (animation.currentFrame > animation.frameCount - 1)
                        animation.currentFrame = (!animation.shouldLoop && animation.hasFinished) ? animation.frameCount - 1 : 0;

                    switch (animation.spriteSheetType)
                    {
                        case SpriteSheetAnimType::Horizontal:
                            spriteRenderer.crop.x = (float)animation.currentFrame * spriteRenderer.crop.width + horizontalOffset;
                            break;

                        case SpriteSheetAnimType::Vertical:
                            spriteRenderer.crop.y = (float)animation.currentFrame * spriteRenderer.crop.height + verticalOffset;
                            break;
                    }
                }

                if (!animation.shouldLoop && animation.currentFrame == animation.frameCount - 1)
                    animation.hasFinished = true;
            }
        }
    }
}
