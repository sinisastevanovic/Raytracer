workspace "Raytracer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Raytracer"

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

include "Haketon/HaketonExternal.lua"
include "Raytracer"
