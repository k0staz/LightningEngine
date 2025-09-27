-- premake5.lua

package.path = package.path .. ";PremakeModules/?.lua"

require "TransitiveLinking"

workspace "LightningEngine"
    architecture "x64"
    configurations {"Debug", "Release"}
    startproject "Application"

    filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8" }

    filter "configurations:Debug"
       defines { "TRACY_ENABLE" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
    include "Engine/Source/EngineBridge/BuildEngineBridge.lua"
    include "Engine/Source/ClientBridge/BuildClientBridge.lua"
    include "Engine/Source/Log/BuildLog.lua"
    include "Engine/Source/Core/BuildCore.lua"
    include "Engine/Source/CoreECS/BuildCoreECS.lua"
    include "Engine/Source/RHI/BuildRHI.lua"
    include "Engine/Source/Renderer/BuildRenderer.lua"
    include "Engine/Source/Client/BuildClient.lua"
    include "Engine/Source/System/BuildSystem.lua"
    include "Engine/Source/Engine/BuildEngine.lua"

group "Windows"
    include "Engine/Source/Windows/D3D11RHI/BuildD3D11.lua"
    include "Engine/Source/Windows/Application/BuildApplication.lua"

group "Application"
    include "Application/BuildApplication.lua"

link_modules()