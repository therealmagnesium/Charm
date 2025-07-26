local output_dir = "%{cfg.buildcfg}-%{cfg.system}/"
local bin_dir = "../../bin/" .. output_dir .. "CharmApp/SandboxProject/assets/scripts/binaries"
local int_dir = "../../bin/" .. output_dir .. "CharmApp/SandboxProject/assets/scripts/intermediates"

workspace("CharmScriptModule")
architecture("x64")
configurations({ "Debug", "Release", "Dist" })
startproject("CharmScriptModule")

project("CharmScriptModule")
kind("SharedLib")
language("C++")
cppdialect("C++17")
systemversion("latest")
pic("on")

files({ "assets/scripts/**.h", "assets/scripts/**.cpp" })

includedirs({
	"assets/scripts",
	"../../Charm/source",
	"../../vendor/glm/",
	"../../vendor/entt/",
	--"vendor/charm/include",
})

libdirs({
	"../../bin/" .. output_dir .. "Charm",
}) -- temp

links({
	"Charm",
})

targetdir(bin_dir)
objdir(int_dir)

filter("system:windows")
defines({ "SANDBOX_PLATFORM_WINDOWS" })

filter("system:linux")
defines({ "SANDBOX_PLATFORM_LINUX" })

filter("configurations:Debug")
defines({ "SANDBOX_DEBUG" })
runtime("Debug")
symbols("On")

filter("configurations:Release")
defines({ "SANDBOX_RELEASE" })
runtime("Release")
optimize("On")
symbols("On")

filter("configurations:Dist")
defines({ "SANDBOX_DIST" })
runtime("Release")
optimize("On")
symbols("Off")
