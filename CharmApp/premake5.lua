project("CharmApp")
kind("ConsoleApp")
language("C++")
cppdialect("C++17")
staticruntime("off")
systemversion("latest")

targetdir("../bin/" .. outputdir .. "/%{prj.name}")
objdir("../build/" .. outputdir .. "/%{prj.name}")

files({
	"source/**.h",
	"source/**.cpp",
})

includedirs({
	"../Charm/source",
	IncludeDir.entt,
	IncludeDir.glad,
	IncludeDir.glm,
	IncludeDir.imgui,
	IncludeDir.ImGuizmo,
})

links({
	"Charm",
	"imgui", -- Temporarly for ImGui::ShowDemoWindow
	--"ImGuizmo",
})

postbuildcommands({
	"{COPYDIR} assets/ %{cfg.buildtarget.directory}",
	"{COPYDIR} SandboxProject/ %{cfg.buildtarget.directory}",
	"{COPY} ../imgui.ini %{cfg.buildtarget.directory}",
})

filter("system:windows")
defines({ "CH_PLATFORM_WINDOWS" })

filter("system:linux")
defines({ "CH_PLATFORM_LINUX" })
links({ "gtk-3" })
linkoptions({ "-Wl,-rpath,/usr/local/lib" })

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
