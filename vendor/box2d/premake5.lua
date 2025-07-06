project("box2d")
kind("StaticLib")
language("C")
cdialect("C17")
systemversion("latest")
pic("on")

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("build/" .. outputdir .. "/%{prj.name}")

files({
    "include/**.h",
    "src/**.h",
    "src/**.c",
    --"shared/**.h",
    --"shared/**.c",
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
