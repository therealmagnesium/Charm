IncludeDir = {}
IncludeDir["SDL3"] = "../vendor/SDL3/include"
IncludeDir["entt"] = "../vendor/entt"
IncludeDir["glad"] = "../vendor/glad/include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["imgui"] = "../vendor/imgui/include"
IncludeDir["stb_image"] = "../vendor/stb_image/include"
IncludeDir["yaml_cpp"] = "../vendor/yaml-cpp/include"

group("Dependencies")
--include("vendor/imgui")
--include("vendor/glad")
--include("vendor/SDL3")
include("vendor/glad")
include("vendor/imgui")
include("vendor/stb_image")
include("vendor/yaml-cpp")
group("")
