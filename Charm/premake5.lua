project("Charm")
kind("StaticLib")
language("C++")
cppdialect("C++17")
staticruntime("off")
systemversion("latest")

files({ "source/**.h", "source/**.cpp" })

includedirs({
    "source",
    "${IncludeDir.SDL3}",
})

links({
    "SDL3",
})

targetdir("../bin/" .. outputdir .. "/%{prj.name}")
objdir("../build/" .. outputdir .. "/%{prj.name}")

filter("system:windows")
defines({ "CH_PLATFORM_WINDOWS" })
libdirs({ "%{LibraryDir.SDL3_Windows}" })

filter("system:linux")
defines({ "CH_PLATFORM_LINUX", "SDL_STATIC_LIB" })
libdirs({ "%{LibraryDir.SDL3_Linux}" })

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
