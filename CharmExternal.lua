IncludeDir = {}
IncludeDir["SDL3"] = "../vendor/SDL3/include"

LibraryDir = {}
LibraryDir["SDL3_Linux"] = "../vendor/SDL3/lib/Linux"
LibraryDir["SDL3_Windows"] = "../vendor/SDL3/lib/Windows"

group("Dependencies")
--include("vendor/imgui")
--include("vendor/glfw")
group("")
