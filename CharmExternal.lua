IncludeDir = {}
IncludeDir["SDL3"] = "../vendor/SDL3/include"
IncludeDir["assimp"] = "../vendor/assimp/include"
IncludeDir["box2d"] = "../vendor/box2d/include"
IncludeDir["entt"] = "../vendor/entt"
IncludeDir["glad"] = "../vendor/glad/include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["imgui"] = "../vendor/imgui/include"
IncludeDir["ImGuizmo"] = "../vendor/ImGuizmo/include"
IncludeDir["nfd"] = "../vendor/nfd/include"
IncludeDir["stb_image"] = "../vendor/stb_image/include"
IncludeDir["yaml_cpp"] = "../vendor/yaml_cpp/include"

group("Dependencies")
--include("vendor/assimp")
include("vendor/box2d")
include("vendor/imgui")
include("vendor/ImGuizmo")
include("vendor/glad")
include("vendor/nfd")
include("vendor/stb_image")
include("vendor/yaml_cpp")
group("")
