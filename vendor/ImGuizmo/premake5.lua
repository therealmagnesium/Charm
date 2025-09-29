project("ImGuizmo")
kind("StaticLib")
language("C++")
cppdialect("C++17")
systemversion("latest")
pic("on")

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("build/" .. outputdir .. "/%{prj.name}")

files({
	"include/*.h",
	"source/*.cpp",
})

includedirs({
	"include",
	"../imgui/include",
})

filter("configurations:Debug")
runtime("Debug")
symbols("on")

filter("configurations:Release")
runtime("Release")
optimize("on")

filter("configurations:Dist")
runtime("Release")
optimize("on")
