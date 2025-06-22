project("stb_image")
kind("StaticLib")
language("C")
cdialect("C17")
systemversion("latest")
pic("on")

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("build/" .. outputdir .. "/%{prj.name}")

files({
	"include/stb_image.h",
	"source/stb_image.c",
})

includedirs({ "include" })

filter("configurations:Debug")
runtime("Debug")
symbols("on")

filter("configurations:Release")
runtime("Release")
optimize("on")

filter("configurations:Dist")
runtime("Release")
optimize("on")
