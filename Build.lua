-- premake5.lua
workspace "LightningEngine"
    architecture "x64"
    configurations {"Debug", "Release"}
    startproject "Application"

    filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
    include "Engine/BuildEngine.lua"

include "Application/BuildApplication.lua"