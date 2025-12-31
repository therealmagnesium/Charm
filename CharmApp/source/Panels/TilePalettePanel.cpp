#include "TilePalettePanel.h"

#include <Core/AssetManager.h>
#include <imgui.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static TilePalettePanelState state;

    namespace TilePalettePanel
    {
        void DrawTilePalettePanel();
        void DrawMode_InvalidEmpty();
        void DrawMode_Invalid();
        void DrawMode_ValidNoTileset();
        void DrawMode_ValidUnsliced();
        void DrawMode_ValidSliced();

        void Display()
        {
            if (!AssetManager::IsHandleValid(state.tilePalette))
            {
                if (AssetManager::IsAssetTypeRegistered(AssetType::TilePalette))
                    state.mode = TilePaletteMode::Invalid;
                else
                    state.mode = TilePaletteMode::InvalidEmpty;
            }

            if (state.shouldDisplay)
                DrawTilePalettePanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }

        void DrawTilePalettePanel()
        {
            ImGui::Begin("Tile Palette", &state.shouldDisplay);

            switch (state.mode)
            {
                case TilePaletteMode::InvalidEmpty:
                    DrawMode_InvalidEmpty();
                    break;
                case TilePaletteMode::Invalid:
                    DrawMode_Invalid();
                    break;
                case TilePaletteMode::ValidNoTileset:
                    DrawMode_ValidNoTileset();
                    break;
                case TilePaletteMode::ValidUnsliced:
                    DrawMode_ValidUnsliced();
                    break;
                case TilePaletteMode::ValidSliced:
                    DrawMode_ValidSliced();
                    break;
                default:
                    break;
            }

            ImGui::End();
        }

        void DrawMode_InvalidEmpty()
        {
            ImGui::TextUnformatted("No tile palettes were found in the asset registry");
            ImGui::Button("Create Palette");
        }

        void DrawMode_Invalid()
        {
            const std::vector<AssetHandle> palettes = AssetManager::GetAllHandlesOfType(AssetType::TilePalette);
            std::string placeholder = "Select a palette";

            if (AssetManager::IsHandleValid(state.tilePalette))
                placeholder = AssetManager::GetAssetPath(state.tilePalette).stem().string();

            const float selectButtonWidth = ImGui::CalcTextSize("Select").x;
            ImGui::TextUnformatted("Select a tile palette to work with");

            ImGui::SetNextItemWidth(-1.3f * selectButtonWidth);
            if (ImGui::BeginCombo("##Tile Palette Selection", placeholder.c_str()))
            {
                if (ImGui::Selectable("None", !AssetManager::IsHandleValid(state.tilePalette)))
                    state.tilePalette = AssetHandle_Invalid;

                for (AssetHandle handle : palettes)
                {
                    const bool isSelected = (state.tilePalette == handle);
                    const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                    if (ImGui::Selectable(stemName.c_str(), isSelected))
                        state.tilePalette = handle;
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();

            if (ImGui::Button("Select"))
            {
                if (AssetManager::IsHandleValid(state.tilePalette))
                {
                    TilePalette* tilePalette = AssetManager::GetAsset<TilePalette>(state.tilePalette);

                    if (AssetManager::IsHandleValid(tilePalette->tileset) && tilePalette->crops.size() > 1)
                        state.mode = TilePaletteMode::ValidSliced;
                    else if (AssetManager::IsHandleValid(tilePalette->tileset))
                        state.mode = TilePaletteMode::ValidUnsliced;
                    else
                        state.mode = TilePaletteMode::ValidNoTileset;
                }
                else
                    ERROR("TilePalettePanel::DrawMode_Invalid - Cannot select an invalid handle for the tile palette!");
            }
        }

        void DrawMode_ValidNoTileset()
        {
            TilePalette* tilePalette = AssetManager::GetAsset<TilePalette>(state.tilePalette);

            const float availableWindowWidth = ImGui::GetContentRegionAvail().x;
            const float backButtonWidth = ImGui::CalcTextSize("Back").x;
            const float selectButtonWidth = ImGui::CalcTextSize("Select").x;

            ImGui::TextUnformatted("Select a tileset to draw and paint with");
            ImGui::SameLine(availableWindowWidth - backButtonWidth);

            if (ImGui::Button("Back"))
            {
                state.mode = TilePaletteMode::Invalid;
                tilePalette->tileset = AssetHandle_Invalid;
            }

            std::string placeholder = "Select a tileset";
            if (AssetManager::IsHandleValid(tilePalette->tileset))
                placeholder = AssetManager::GetAssetPath(tilePalette->tileset).stem().string();

            ImGui::SetNextItemWidth(-selectButtonWidth * 1.3f);
            if (ImGui::BeginCombo("##Tileset Selection", placeholder.c_str()))
            {
                if (ImGui::Selectable("None", !AssetManager::IsHandleValid(tilePalette->tileset)))
                    tilePalette->tileset = AssetHandle_Invalid;

                const std::vector<AssetHandle> tilesets = AssetManager::GetAllHandlesOfType(AssetType::Texture);
                for (AssetHandle handle : tilesets)
                {
                    const bool isSelected = (tilePalette->tileset == handle);
                    const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                    if (ImGui::Selectable(stemName.c_str(), isSelected))
                        tilePalette->tileset = handle;
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();

            if (ImGui::Button("Select"))
            {
                if (AssetManager::IsHandleValid(tilePalette->tileset))
                    state.mode = TilePaletteMode::ValidUnsliced;
                else
                    ERROR("TilePalettePanel::Display - Cannot selct an invalid handle for the tileset!");
            }

            if (AssetManager::IsHandleValid(tilePalette->tileset))
            {
                Texture* tileset = AssetManager::GetAsset<Texture>(tilePalette->tileset);

                const float targetAspect = (float)tileset->width / (float)tileset->height;
                ImVec2 regionSize = ImGui::GetContentRegionAvail();
                ImVec2 aspectSize = ImVec2(regionSize.x, regionSize.x / targetAspect);

                if (aspectSize.y > regionSize.y)
                {
                    aspectSize.y = regionSize.y;
                    aspectSize.x = regionSize.y * targetAspect;
                }

                ImGui::Image(tileset->id, aspectSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));
            }
        }

        void DrawMode_ValidUnsliced() { ImGui::Text("PLACEHOLDER: VALID UNSLICED"); }
        void DrawMode_ValidSliced() { ImGui::Text("PLACEHOLDER: VALID SLICED"); }
    }
}
