#include "Graphics/Animation.h"
#include "Graphics/Texture.h"

#include "Core/AssetManager.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include "Projects/Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

using namespace Charm::Core;
using namespace Charm::Projects;

namespace Charm::Graphics
{
    namespace Animations
    {
        Animation Load(const char* path)
        {
            Animation animation;

            YAML::Node data;
            try
            {
                data = YAML::LoadFile(path);
            }
            catch (const YAML::BadFile& e)
            {
                ERROR("Animations::Load - Could not load the file \"%s\"", path);
                return animation;
            }

            ASSERT_RETURN(data, animation, "Animations::Load - Failed to load animation \"%s\", the path may be invalid!", path);
            ASSERT_RETURN(data["Animation"], animation, "Animations::Load - Failed to load animation \"%s\", the path may be an invalid animation file!", path);

            const YAML::Node& properties = data["Properties"];
            animation.speed = properties["Speed"].as<u32>();
            animation.shouldLoop = properties["Should Loop"].as<bool>();
            animation.frameCount = properties["Frame Count"].as<u32>();

            if (animation.frameCount > 0)
                animation.frames.resize(animation.frameCount);

            const YAML::Node& framesNode = properties["Frames"];

            u32 frameIndex = 0;
            for (const auto& frameEntry : framesNode)
            {
                const YAML::Node& frameNode = frameEntry["Frame " + std::to_string(frameIndex)];

                Rectangle& frame = animation.frames[frameIndex];
                frame.x = frameNode["X"].as<float>();
                frame.y = frameNode["Y"].as<float>();
                frame.width = frameNode["Width"].as<float>();
                frame.height = frameNode["Height"].as<float>();

                frameIndex++;
            }

            // animation.rowOffset = properties["Row Offset"].as<u32>();
            // animation.columnOffset = properties["Column Offset"].as<u32>();
            // animation.spriteSheetType = Utils::StringToSpriteSheetAnimType(properties["Sprite Sheet Type"].as<std::string>());
            animation.isValid = true;

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
            out << YAML::Key << "Should Loop" << YAML::Value << animation.shouldLoop;
            out << YAML::Key << "Frame Count" << YAML::Value << animation.frameCount;

            out << YAML::Key << "Frames" << YAML::Value << YAML::BeginSeq;

            for (u32 i = 0; i < animation.frames.size(); i++)
            {
                out << YAML::BeginMap;

                out << YAML::Key << "Frame " + std::to_string(i) << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "X" << YAML::Value << animation.frames[i].x;
                out << YAML::Key << "Y" << YAML::Value << animation.frames[i].y;
                out << YAML::Key << "Width" << YAML::Value << animation.frames[i].width;
                out << YAML::Key << "Height" << YAML::Value << animation.frames[i].height;
                out << YAML::EndMap;

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap;

            // out << YAML::Key << "Row Offset" << YAML::Value << animation.rowOffset;
            // out << YAML::Key << "Column Offset" << YAML::Value << animation.columnOffset;
            // out << YAML::Key << "Sprite Sheet Type" << YAML::Value << Utils::SpriteSheetAnimTypeToString(animation.spriteSheetType);

            std::ofstream fout(path);
            if (fout.is_open())
            {
                fout << out.c_str() << "\n";
                fout.close();

                INFO("Animation \"%s\" was saved successfully", path);
            }
            else
                ERROR("Animations::Save - Failed to save animation \"%s\", the path may be invalid!", path);
        }

        void Reset(Animation& animation)
        {
            animation.currentFrame = 0;
            animation.counter = 0;
            animation.hasFinished = false;
        }

        void Update(Animation& animation)
        {
            if (animation.speed == 0)
                return;

            animation.counter++;

            if (animation.counter >= (60 / animation.speed))
            {
                animation.counter = 0;
                animation.currentFrame++;

                if (animation.currentFrame > animation.frameCount - 1)
                    animation.currentFrame = (!animation.shouldLoop && animation.hasFinished) ? animation.frameCount - 1 : 0;
            }

            if (!animation.shouldLoop && animation.currentFrame == animation.frameCount - 1)
                animation.hasFinished = true;
        }

        void Apply(Animation& animation, Rectangle& rect, const Texture& texture)
        {
            if (animation.frames.size() < 1)
                return;

            rect = animation.frames[animation.currentFrame];

            // TODO: Replace aLl of this shit with the line above so the user can use any frame from the sprite sheet
            /*
                        rect.width = (float)texture.width / texture.columnCount;
                        rect.height = (float)texture.height / texture.rowCount;

                        const u32 horizontalOffset = animation.columnOffset * rect.width;
                        const u32 verticalOffset = animation.rowOffset * rect.height;

                        switch (animation.spriteSheetType)
                        {
                            case SpriteSheetAnimType::Horizontal:
                                rect.x = (float)animation.currentFrame * rect.width + horizontalOffset;
                                break;

                            case SpriteSheetAnimType::Vertical:
                                rect.y = (float)animation.currentFrame * rect.height + verticalOffset;
                                break;

                            default:
                                break;*/
        }

        AnimationController LoadController(const char* path)
        {
            AnimationController controller;
            YAML::Node data;

            try
            {
                data = YAML::LoadFile(path);
            }
            catch (const YAML::BadFile& e)
            {
                ERROR("Animations::LoadController - Could not load the file \"%s\"", path);
                return controller;
            }

            ASSERT_RETURN(data, controller, "Animations::LoadController - Failed to load animation controller \"%s\", the path may be invalid!", path);
            ASSERT_RETURN(data["Animation Controller"], controller, "Animations::LoadController - Failed to load animation controller \"%s\", the path may be an invalid animation file!", path);

            u32 animationCount = data["Animation Count"].as<u32>();

            if (animationCount > 0)
            {
                controller.animations.reserve(animationCount);
                const YAML::Node& animations = data["Animations"];

                const Project& project = ProjectManager::GetActive();
                for (auto animationNode : animations)
                {
                    const YAML::Node& animation = animationNode["Animation"];
                    const AssetHandle animHandle = animation["Handle"].as<AssetHandle>();
                    const std::filesystem::path animSavedPath = animation["Path"].as<std::string>();

                    if (AssetManager::IsHandleValid(animHandle))
                    {
                        controller.animations.emplace_back(animHandle);
                        continue;
                    }

                    AssetManager::Import(animSavedPath, AssetType::Animation, animHandle);
                    controller.animations.emplace_back(animHandle);
                }
            }

            controller.isValid = true;
            INFO("Animation Controller \"%s\" was loaded successfully", path);
            return controller;
        }

        void SaveController(const char* path, const AnimationController& controller)
        {
            YAML::Emitter out;
            out << YAML::BeginMap; // Root

            out << YAML::Key << "Animation Controller" << YAML::Value << controller.handle;
            out << YAML::Key << "Animation Count" << YAML::Value << controller.animations.size();

            out << YAML::Key << "Animations" << YAML::Value << YAML::BeginSeq;

            for (AssetHandle animation : controller.animations)
            {
                const std::filesystem::path animPath = AssetManager::GetAssetPath(animation);

                out << YAML::BeginMap;

                out << YAML::Key << "Animation" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << animation;
                out << YAML::Key << "Path" << YAML::Value << animPath;
                out << YAML::EndMap;

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream fout(path);
            if (!fout.is_open())
            {
                ERROR("Animations::SaveController - Failed to save animation controller \"%s\", the path may be invalid!", path);
                return;
            }

            fout << out.c_str() << "\n";
            fout.close();

            INFO("Animation Controller \"%s\" was saved successfully", path);
        }

    }
}
