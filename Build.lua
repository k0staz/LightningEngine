-- premake5.lua

package.path = package.path .. ";PremakeModules/?.lua"

require "TransitiveLinking"

workspace "LightningEngine"
    architecture "x64"
    configurations {"Debug", "Release"}
    startproject "Application"

    filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
    include "Engine/Source/Log/BuildLog.lua"
    include "Engine/BuildEngine.lua"

group "Application"
    include "Application/BuildApplication.lua"

link_modules()