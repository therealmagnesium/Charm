IncludeDir = {}
IncludeDir["SDL3"] = "../vendor/SDL3/include"
IncludeDir["glad"] = "../vendor/glad/include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["imgui"] = "../vendor/imgui/include"
IncludeDir["stb_image"] = "../vendor/stb_image/include"

LibraryDir = {}
LibraryDir["SDL3_Linux"] = "../vendor/SDL3/lib/Linux"
LibraryDir["SDL3_Windows"] = "../vendor/SDL3/lib/Windows"
LibraryDir["glad_Linux"] = "../vendor/glad/lib/Linux"
LibraryDir["glad_Windows"] = "../vendor/glad/lib/Windows"
LibraryDir["imgui_Linux"] = "../vendor/imgui/lib/Linux"
LibraryDir["imgui_Windows"] = "../vendor/imgui/lib/Windows"
LibraryDir["stb_image_Linux"] = "../vendor/stb_image/lib/Linux"
LibraryDir["stb_image_Windows"] = "../vendor/stb_image/lib/Windows"

group("Dependencies")
--include("vendor/imgui")
--include("vendor/glfw")
group("")
