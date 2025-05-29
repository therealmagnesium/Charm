project("CharmApp")
kind("ConsoleApp")
language("C++")
cppdialect("C++17")
staticruntime("off")
systemversion("latest")

files({ "source/**.h", "source/**.cpp" })

includedirs({
	"../Charm/source",
	IncludeDir.SDL3,
	IncludeDir.glad,
	IncludeDir.glm,
})

links({
	"Charm",
	"SDL3",
	"glad",
})

targetdir("../bin/" .. outputdir .. "/%{prj.name}")
objdir("../build/" .. outputdir .. "/%{prj.name}")

filter("system:windows")
defines({ "CH_PLATFORM_WINDOWS" })
libdirs({
	LibraryDir.SDL3_Windows,
	LibraryDir.glad_Windows,
})

filter("system:linux")
defines({ "CH_PLATFORM_LINUX" })
libdirs({
	LibraryDir.SDL3_Linux,
	LibraryDir.glad_Linux,
})

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
kind("WindowedApp")
defines({ "CH_DIST" })
runtime("Release")
optimize("On")
symbols("Off")
