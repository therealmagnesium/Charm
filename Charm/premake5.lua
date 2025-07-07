project("Charm")
kind("SharedLib")
language("C++")
cppdialect("C++17")
systemversion("latest")
pic("on")

files({ "source/**.h", "source/**.cpp" })

includedirs({
	"source",
	--IncludeDir.SDL3,
	IncludeDir.box2d,
	IncludeDir.entt,
	IncludeDir.glad,
	IncludeDir.glm,
	IncludeDir.imgui,
	IncludeDir.nfd,
	IncludeDir.stb_image,
	IncludeDir.yaml_cpp,
})

links({
	"SDL3",
	"box2d",
	"glad",
	"imgui",
	"native-file-dialog",
	"stb_image",
	"yaml-cpp",
})

targetdir("../bin/" .. outputdir .. "/%{prj.name}")
objdir("../build/" .. outputdir .. "/%{prj.name}")

filter("system:windows")
defines({ "CH_PLATFORM_WINDOWS" })
libdirs({})

filter("system:linux")
defines({ "CH_PLATFORM_LINUX", "SDL_STATIC_LIB" })
libdirs({})

filter("configurations:Debug")
defines({ "CH_DEBUG" })
runtime("Debug")
symbols("On")

filter("configurations:Release")
defines({ "CH_RELEASE" })
runtime("Release")
optimize("On")
symbols("On")

filter("configurations:Dist")
defines({ "CH_DIST" })
runtime("Release")
optimize("On")
symbols("Off")
