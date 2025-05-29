workspace("Charm")
architecture("x64")
configurations({ "Debug", "Release", "Dist" })
startproject("CharmApp")

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

include("CharmExternal.lua")
include("Charm")
include("CharmApp")
