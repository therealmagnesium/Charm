#include "AssetRegistryPanel.h"

#include <Core/AssetManager.h>
#include <Core/Random.h>
#include <Core/Utils.h>
#include <imgui.h>

using namespace Charm;
using namespace Charm::Core;

namespace CharmApp
{
    namespace AssetRegistryPanel
    {
        void Display()
        {
            ImGui::Begin("Asset Registry");
            const float columnWidth = 65.f;
            for (auto& [handle, metadata] : AssetManager::GetRegistry())
            {
                ImGui::PushID(handle);

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Handle");
                ImGui::NextColumn();
                ImGui::Text("0x%lx", handle);
                ImGui::Columns(1);
                ImGui::Separator();

                ImGui::PopID();

                ImGui::PushID(metadata.path.c_str());

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Path");
                ImGui::NextColumn();
                ImGui::Text("%s", metadata.path.c_str());
                ImGui::Columns(1);
                ImGui::Separator();

                ImGui::PopID();

                ImGui::PushID(("Type" + std::to_string(handle)).c_str());

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Type");
                ImGui::NextColumn();
                ImGui::Text("%s", "Texture");
                ImGui::Columns(1);
                ImGui::Separator();

                ImGui::PopID();
            }
            ImGui::End();
        }
    }
}
