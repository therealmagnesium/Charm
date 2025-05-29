IncludeDir = {}
IncludeDir["SDL3"] = "../vendor/SDL3/include"
IncludeDir["glad"] = "../vendor/glad/include"
IncludeDir["glm"] = "../vendor/glm"

LibraryDir = {}
LibraryDir["SDL3_Linux"] = "../vendor/SDL3/lib/Linux"
LibraryDir["SDL3_Windows"] = "../vendor/SDL3/lib/Windows"
LibraryDir["glad_Linux"] = "../vendor/glad/lib/Linux"
LibraryDir["glad_Windows"] = "../vendor/glad/lib/Windows"

group("Dependencies")
--include("vendor/imgui")
--include("vendor/glfw")
group("")
