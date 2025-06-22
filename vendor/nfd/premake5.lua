project("native-file-dialog")
kind("StaticLib")
language("C++")
cppdialect("C++17")
systemversion("latest")
pic("on")

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("build/" .. outputdir .. "/%{prj.name}")

files({
	"include/**.h",
	"include/**.hpp",
})

includedirs({ "include" })

filter("system:linux")
includedirs({ "/usr/include/atk-1.0" })
includedirs({ "/usr/include/cairo" })
includedirs({ "/usr/include/gdk-pixbuf-2.0" })
includedirs({ "/usr/include/gtk-3.0" })
includedirs({ "/usr/include/glib-2.0" })
includedirs({ "/usr/include/harfbuzz" })
includedirs({ "/usr/include/pango-1.0" })
files({ "src/nfd_gtk.cpp" })

filter("system:windows")
files({ "src/nfd_win.cpp" })

filter("configurations:Debug")
runtime("Debug")
symbols("on")

filter("configurations:Release")
runtime("Release")
optimize("on")

filter("configurations:Dist")
runtime("Release")
optimize("on")
