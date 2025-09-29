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
        enum ColumnID
        {
            Index = 0,
            Type,
            Path,
            Handle
        };

        static AssetRegistryState state;

        void Init()
        {
            state.flags = ImGuiTableFlags_Resizable |
                          ImGuiTableFlags_Reorderable |
                          ImGuiTableFlags_Hideable |
                          ImGuiTableFlags_Sortable |
                          ImGuiTableFlags_SortMulti |
                          ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_BordersOuter |
                          ImGuiTableFlags_BordersV |
                          ImGuiTableFlags_NoBordersInBody |
                          ImGuiTableFlags_ScrollY;
        }

        void Display()
        {
            ImGui::Begin("Asset Registry");

            if (ImGui::BeginTable("Asset Registry Table", 3, state.flags))
            {
                // ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, ColumnID::Index);
                ImGui::TableSetupColumn("Type", 0, 0.0f, ColumnID::Type);
                ImGui::TableSetupColumn("Path", 0, 0.0f, ColumnID::Path);
                ImGui::TableSetupColumn("Handle", 0, 0.0f, ColumnID::Handle);
                ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                ImGui::TableHeadersRow();

                /*
                        // Sort our data if sort specs have been changed!
                        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
                        {
                            if (sortSpecs->SpecsDirty)
                            {

                                MyItem::SortWithSortSpecs(sortSpecs, items.Data, items.Size);
                                sort_specs->SpecsDirty = false;
                            }
                        }*/

                const AssetRegistry& assetRegistry = AssetManager::GetRegistry();
                for (auto& [handle, metadata] : assetRegistry)
                {
                    ImGui::PushID(handle);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(Utils::AssetTypeToString(metadata.type).c_str());
                    ImGui::SameLine();
                    ImGui::SmallButton("x");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(metadata.path.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%lX", handle);
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::End();
        }
    }
}
